#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/fpu.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Handles for RTOS Primitives
static SemaphoreHandle_t xButtonSemaphore = NULL; // Binary semaphore for SW1 button
static QueueHandle_t     xRxQueue          = NULL; // Queue for incoming UART characters
static SemaphoreHandle_t xUartMutex        = NULL; // Mutex to serialize UART TX printing

// -----------------------------------------------------------------------------
// 1. THREAD-SAFE UART TRANSMIT FUNCTION
// -----------------------------------------------------------------------------
void UART_SafePrint(const char *str) {
    if (xSemaphoreTake(xUartMutex, portMAX_DELAY) == pdTRUE) {
        while (*str) {
            UARTCharPut(UART0_BASE, *str++);
        }
        xSemaphoreGive(xUartMutex);
    }
}

// -----------------------------------------------------------------------------
// 2. HARDWARE INTERRUPT HANDLERS
// -----------------------------------------------------------------------------

// GPIO Port F Handler (SW1 Button Press on PF4)
void GPIOPortF_Handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Clear interrupt flag on PF4
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);

    // Dummy read to flush the ARM Cortex-M write buffer (prevents double interrupt)
    (void)GPIOIntStatus(GPIO_PORTF_BASE, true);

    // Give binary semaphore to wake up vButtonTask
    xSemaphoreGiveFromISR(xButtonSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// UART0 Handler (Receives keyboard input from PC terminal)
void UART0_Handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    uint32_t status = UARTIntStatus(UART0_BASE, true);
    UARTIntClear(UART0_BASE, status);

    while (UARTCharsAvail(UART0_BASE)) {
        char c = UARTCharGetNonBlocking(UART0_BASE);
        xQueueSendFromISR(xRxQueue, &c, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// -----------------------------------------------------------------------------
// 3. FREERTOS TASKS
// -----------------------------------------------------------------------------

// TASK 1: Button Handler Task (Triggered by SW1 press)
void vButtonTask(void *pvParameters) {
    for (;;) {
        // Sleep until SW1 button is pressed
        if (xSemaphoreTake(xButtonSemaphore, portMAX_DELAY) == pdTRUE) {
            
            // Toggle Blue LED (PF2)
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 
                        GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2) ^ GPIO_PIN_2);

            // Report button press over UART
            UART_SafePrint("[BUTTON]: SW1 Pressed! Blue LED toggled.\r\n");

            // Debounce delay: ignore noise for 200ms
            vTaskDelay(pdMS_TO_TICKS(200));

            // Clear any lingering bounce interrupts during delay
            xSemaphoreTake(xButtonSemaphore, 0);
        }
    }
}

// TASK 2: Consumes Incoming UART Characters from Terminal
void vRxCommandTask(void *pvParameters) {
    char rxChar;

    for (;;) {
        // Sleep until UART ISR pushes a character into the queue
        if (xQueueReceive(xRxQueue, &rxChar, portMAX_DELAY) == pdPASS) {
            
            if (rxChar == '1') {
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // Green LED ON
                UART_SafePrint("[CMD]: Green LED turned ON\r\n");
            } 
            else if (rxChar == '0') {
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);          // Green LED OFF
                UART_SafePrint("[CMD]: Green LED turned OFF\r\n");
            } 
            else {
                // Echo unrecognized keys
                UART_SafePrint("[ECHO]: ");
                char echoStr[2] = {rxChar, '\0'};
                UART_SafePrint(echoStr);
                UART_SafePrint("\r\n");
            }
        }
    }
}

// TASK 3: Periodic System Log
void vHeartbeatTask(void *pvParameters) {
    for (;;) {
        // Toggle Red LED (PF1)
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 
                    GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) ^ GPIO_PIN_1);

        // Print system heartbeat log
        UART_SafePrint("[SYSTEM]: Heartbeat Active...\r\n");

        vTaskDelay(pdMS_TO_TICKS(3000)); // Sleep for 3 seconds
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    taskDISABLE_INTERRUPTS();
    for (;;);
}

// -----------------------------------------------------------------------------
// 4. MAIN ENTRY POINT & HARDWARE CONFIGURATION
// -----------------------------------------------------------------------------
int main(void) {
    FPUEnable();
    FPULazyStackingEnable();

    // Set CPU Clock to 80 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Enable Hardware Peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0) || !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    // Configure UART0 Pins (PA0 Rx, PA1 Tx)
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Set UART0 to 115200 Baud, 8 Data Bits, 1 Stop Bit, No Parity
    UARTConfigSetExpClk(UART0_BASE, 80000000, 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    // Configure LED Outputs (PF1 Red, PF2 Blue, PF3 Green)
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0);

    // Configure SW1 Input (PF4) WITH PULL-UP (Type set first, then Pull-Up applied)
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // Create RTOS Primitives
    xButtonSemaphore = xSemaphoreCreateBinary();
    xRxQueue          = xQueueCreate(16, sizeof(char));
    xUartMutex        = xSemaphoreCreateMutex();

    if (xButtonSemaphore != NULL && xRxQueue != NULL && xUartMutex != NULL) {
        // Configure GPIO Interrupt for SW1 (PF4)
        GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);
        GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4);
        IntPrioritySet(INT_GPIOF, (5 << 5));
        IntEnable(INT_GPIOF);

        // Configure UART Interrupts (RX and Receive Timeout)
        UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
        IntPrioritySet(INT_UART0, (5 << 5));
        IntEnable(INT_UART0);

        // Create Tasks
        xTaskCreate(vButtonTask,    "ButtonTask", 256, NULL, 2, NULL);
        xTaskCreate(vRxCommandTask, "RxTask",     256, NULL, 2, NULL);
        xTaskCreate(vHeartbeatTask, "LogTask",    256, NULL, 1, NULL);

        // Initial Greeting
        UART_SafePrint("\r\n=== UART + GPIO FreeRTOS Driver Ready ===\r\n");
        UART_SafePrint("Press SW1 on board to toggle Blue LED\r\n");
        UART_SafePrint("Press '1' on terminal to turn Green LED ON\r\n");
        UART_SafePrint("Press '0' on terminal to turn Green LED OFF\r\n\r\n");

        // Start RTOS Scheduler
        vTaskStartScheduler();
    }

    while (1);
}
