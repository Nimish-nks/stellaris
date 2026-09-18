#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/fpu.h"

#include "FreeRTOS.h"
#include "task.h"

// Task 1: Toggles Red LED (PF1) every 500 ms
void vRedLedTask(void *pvParameters) {
    for (;;) {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 
                    GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) ^ GPIO_PIN_1);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Task 2: Toggles Blue LED (PF2) every 200 ms
void vBlueLedTask(void *pvParameters) {
    for (;;) {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 
                    GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2) ^ GPIO_PIN_2);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    taskDISABLE_INTERRUPTS();
    for (;;);
}

int main(void) {
    // 1. Enable Hardware FPU
    FPUEnable();
    FPULazyStackingEnable();

    // 2. Set Clock to 80 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // 3. Enable Port F (PF1 Red, PF2 Blue)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2, 0);

    // 4. Register equal priority tasks (Priority 1)
    xTaskCreate(vRedLedTask,  "RedLED",  128, NULL, 1, NULL);
    xTaskCreate(vBlueLedTask, "BlueLED", 128, NULL, 1, NULL);

    // 5. Hand over CPU control to FreeRTOS Scheduler
    vTaskStartScheduler();

    while(1);
}