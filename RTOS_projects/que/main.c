#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/fpu.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"  //freertos header containing quecreate,send and receive functions

static QueueHandle_t xLedQueue = NULL;

void vProducerTask(void *pvParameters) {
    uint32_t ledState = 1;
    for (;;) {
        // Send state (1 or 0) into the queue; wait up to 10 ticks if full
        xQueueSend(xLedQueue, &ledState, pdMS_TO_TICKS(10));
        ledState ^= 1; // Toggle state for next iteration
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vConsumerTask(void *pvParameters) {
    uint32_t receivedState = 0;
    for (;;) {
        // Block indefinitely (portMAX_DELAY) until data arrives in the queue
        if (xQueueReceive(xLedQueue, &receivedState, portMAX_DELAY) == pdPASS) {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, receivedState ? GPIO_PIN_1 : 0);
        }
    }
}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    // Trap execution here if a task exceeds its allocated stack space
    taskDISABLE_INTERRUPTS();
    for (;;);
}

int main(void) {
    FPUEnable();
    FPULazyStackingEnable();
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);

    // Create a FIFO queue holding up to 5 elements of type uint32_t
    xLedQueue = xQueueCreate(5, sizeof(uint32_t));

    if (xLedQueue != NULL) {
        xTaskCreate(vProducerTask, "Producer", 128, NULL, 1, NULL);
        xTaskCreate(vConsumerTask, "Consumer", 128, NULL, 2, NULL); // Higher priority consumer
        vTaskStartScheduler();
    }
    while (1);
}

