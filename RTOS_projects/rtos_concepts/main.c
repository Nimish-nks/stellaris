#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/fpu.h"
#include "driverlib/interrupt.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Struct sent through the Queue
typedef struct {
    uint32_t pressCount;
} ButtonEvent_t;

// Handles for the 4 Pillars
static SemaphoreHandle_t xButtonSemaphore = NULL; // 2. SEMAPHORE
static QueueHandle_t     xDataQueue       = NULL; // 3. QUEUE
static SemaphoreHandle_t xSharedMutex     = NULL; // 4. MUTEX

// Hardware Interrupt Routine
void GPIOPortF_Handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);

    // PILLAR 2: Signal the waiting task from ISR
    xSemaphoreGiveFromISR(xButtonSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// PILLAR 1 - TASK 1: Button Handler Task
void vButtonTask(void *pvParameters) {
    ButtonEvent_t event;
    event.pressCount = 0;

    for (;;) {
        // Wait for ISR signal
        if (xSemaphoreTake(xButtonSemaphore, portMAX_DELAY) == pdTRUE) {
            event.pressCount++;

            // PILLAR 3: Send event structure into the Queue
            xQueueSend(xDataQueue, &event, pdMS_TO_TICKS(10));
        }
    }
}

// PILLAR 1 - TASK 2: Heavy Data Processing Task
void vProcessingTask(void *pvParameters) {
    ButtonEvent_t receivedEvent;

    for (;;) {
        // PILLAR 3: Wait for data from Queue
        if (xQueueReceive(xDataQueue, &receivedEvent, portMAX_DELAY) == pdPASS) {
            
            // PILLAR 4: Lock Shared Resource (Green LED PF3)
            if (xSemaphoreTake(xSharedMutex, portMAX_DELAY) == pdTRUE) {
                // Simulate processing heavy work
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
                vTaskDelay(pdMS_TO_TICKS(300)); 
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);

                // Release Mutex
                xSemaphoreGive(xSharedMutex);
            }
        }
    }
}

// PILLAR 1 - TASK 3: Periodic Background Heartbeat
void vHeartbeatTask(void *pvParameters) {
    for (;;) {
        // Toggle Red LED (PF1) as background system check
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 
                    GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) ^ GPIO_PIN_1);

        // PILLAR 4: Safely access Shared Resource (Blue LED PF2)
        if (xSemaphoreTake(xSharedMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
            vTaskDelay(pdMS_TO_TICKS(100));
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

            // Release Mutex
            xSemaphoreGive(xSharedMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    taskDISABLE_INTERRUPTS();
    for (;;);
}

int main(void) {
    FPUEnable();
    FPULazyStackingEnable();

    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    // Configure LEDs (PF1 Red, PF2 Blue, PF3 Green)
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0);

    // Configure SW1 (PF4) Input
    
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    // 1. ALLOCATE RTOS OBJECTS
    xButtonSemaphore = xSemaphoreCreateBinary();
    xDataQueue       = xQueueCreate(5, sizeof(ButtonEvent_t));
    xSharedMutex     = xSemaphoreCreateMutex();

    if (xButtonSemaphore != NULL && xDataQueue != NULL && xSharedMutex != NULL) {
        // Configure GPIO Interrupts
        GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);
        GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4);
        IntPrioritySet(INT_GPIOF, (5 << 5));
        IntEnable(INT_GPIOF);

        // 2. CREATE TASKS (PILLAR 1)
        xTaskCreate(vHeartbeatTask,  "Heartbeat",  128, NULL, 1, NULL);
        xTaskCreate(vButtonTask,     "ButtonTask", 128, NULL, 2, NULL);
        xTaskCreate(vProcessingTask, "ProcessTask",128, NULL, 2, NULL);

        // Start Kernel
        vTaskStartScheduler();
    }

    while (1);
}