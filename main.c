#include "gpio.h"

void delay(void) {
    for (volatile int i = 0; i < 1000000; i++);
}

int main(void) {
   gpio_init();
   while(1)  {
      gpio_toggle();
      delay();
   }
}
