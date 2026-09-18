#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/fpu.h"
#include "FreeRTOS.h"
#include "task.h"

void vTestTask(void *pvParameters) {
    // 1. Turn Red LED OFF immediately upon entering the task
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    for (;;) {
        // Toggle Red LED (PF1)
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 
                    GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) ^ GPIO_PIN_1);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    // Traps execution here if a task exceeds its allocated stack space
    taskDISABLE_INTERRUPTS();
    for (;;);
}
int main(void) {
    // --- Step 1: Immediate Hardware Sanity Test ---
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); // Force Red LED ON immediately
    // 1. MUST enable hardware FPU first for FreeRTOS ARM_CM4F port
    FPUEnable();
    FPULazyStackingEnable();
    // 80 MHz System Clock
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);

    xTaskCreate(vTestTask, "Test", 128, NULL, 1, NULL);
    vTaskStartScheduler();

    while(1); // Should never reach here
}