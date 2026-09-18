#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"

// High Priority Exception Handler (Priority 1 / 0x20)
void SysTickHandler(void) {
    // Toggle Blue LED (PF2)
    uint32_t val = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, val ^ GPIO_PIN_2);
}

// Low Priority IRQ Handler (Priority 2 / 0x40)
void GPIOPortFHandler(void) {
    // 1. Instantly disable SW1 interrupts to block mechanical bounce signals
    GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_4);

    // --- DEMO 1: PREEMPTION ---
    // Turn ON Red LED (PF1). SysTick (Blue LED) keeps blinking!
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
    SysCtlDelay(2666666); // ~0.5 second delay

    // --- DEMO 2: BASEPRI MASKING ---
    // Block interrupts with priority <= 0x20 (Masks SysTick!)
    IntPriorityMaskSet(0x20);

    // Turn OFF Red LED (PF1) and Turn ON Green LED (PF3)
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_3, GPIO_PIN_3);
    
    // Blue LED stops blinking during this delay
    SysCtlDelay(10666666); // ~2.0 second delay

    // Unmask BASEPRI back to 0 (Allow SysTick again)
    IntPriorityMaskSet(0x00);

    // Turn OFF Green LED (PF3)
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);

    // 2. Wipe out any bounce flags that latched while processing
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);

    // 3. Safe to re-enable SW1 interrupts now that button contacts have settled
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4);
}

int main(void) {
    // 1. Clock Setup (16 MHz)
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // 2. Enable Port F Peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    // Configure Outputs: Red (PF1), Blue (PF2), Green (PF3)
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0);

    // Configure Input: SW1 (PF4) with Pull-Up
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // --- SET NVIC INTERRUPT PRIORITIES ---
    IntPrioritySet(FAULT_SYSTICK, 0x20); // SysTick = Priority 1 (0x20) -> Higher
    IntPrioritySet(INT_GPIOF, 0x40);      // GPIO Port F = Priority 2 (0x40) -> Lower

    // --- CONFIGURE INTERRUPT SOURCES ---
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4);
    IntEnable(INT_GPIOF);

    // Configure SysTick to fire every ~200ms (16MHz * 0.2s = 3,200,000 cycles)
    SysTickPeriodSet(3200000);
    SysTickIntEnable();
    SysTickEnable();

    // Enable CPU Global Interrupts
    IntMasterEnable();

    while(1) {
        __asm(" wfi "); // Low power sleep loop
    }
}