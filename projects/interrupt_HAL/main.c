#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"

// Interrupt Service Routine for GPIO Port F
// Must match prototype declared in startup_gcc.c vector table
void GPIOPortFHandler(void) {
    // Read the masked interrupt status to identify active interrupt flags on Port F
    uint32_t status = GPIOIntStatus(GPIO_PORTF_BASE, true);
    
    // MUST clear the hardware flag; failing to do this causes endless ISR looping
    GPIOIntClear(GPIO_PORTF_BASE, status);
    
    // Read current state of PF1 (Red LED) and toggle it using XOR
    uint32_t currentVal = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, currentVal ^ GPIO_PIN_1);
}

int main(void) {
    // Set system clock to 80 MHz (200MHz PLL / 2.5 = 80MHz using 16MHz main crystal)
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Power on GPIO Port F peripheral bus
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    
    // Wait until the peripheral is ready to accept register writes to avoid HardFault
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    // Configure Port F Pin 1 (Red LED) as a digital output
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
    
    // Ensure Red LED starts in OFF state
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);

    // Configure Port F Pin 4 (SW1 button) as a digital input
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
    
    // Enable internal weak pull-up resistor on PF4 (button connects signal to GND when pressed)
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // --- GATE 1: Peripheral Level Setup ---
    // Set PF4 interrupt trigger to falling edge (HIGH -> LOW transition when pressed)
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);
    
    // Enable interrupt output specifically for Pin 4 inside GPIO Port F module
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4);

    // --- GATE 2: NVIC Level Setup ---
    // Unmask GPIO Port F vector line in central Cortex-M NVIC controller
    IntEnable(INT_GPIOF);

    // --- GATE 3: CPU Global Level Setup ---
    // Enable CPU master interrupts (clears CPU PRIMASK register)
    IntMasterEnable();

    // Sleep loop: CPU enters low-power state and wakes only when interrupt arrives
    while(1) {
        __asm(" wfi ");
    }
}
