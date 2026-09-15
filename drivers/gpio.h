#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

/*base address for gpio port F for inbuilt LED*/

#define GPIO_PORTF_BASE 0x40025000
#define SYSCTL_RCGCGPIO_R (*(volatile uint32_t *)0x400FE608)

/*offset for registers*/
#define GPIO_DIR (*((volatile uint32_t *)(GPIO_PORTF_BASE + 0x400)))
#define GPIO_DEN (*((volatile uint32_t *)(GPIO_PORTF_BASE + 0x51C)))
#define GPIO_DATA (*((volatile uint32_t *)(GPIO_PORTF_BASE + 0x3FC)))
/*FUNCTION PROTOTYPES*/
void gpio_init(void);
void gpio_toggle(void);

#endif
