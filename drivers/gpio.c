#include "gpio.h"
void gpio_init(void) {
    SYSCTL_RCGCGPIO_R |= (1 << 5);
    GPIO_DIR |= (1 << 1);  // set red  led as output
    GPIO_DEN |= (1 << 1);  // enable digital function
}

void gpio_toggle(void) {
    GPIO_DATA ^= (1 << 1);
}
