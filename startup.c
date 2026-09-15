#include <stdint.h>

/*Symbols defined in linker script*/
extern uint32_t _estack; //top of stack
extern uint32_t _sidata; // start of init values in flash
extern uint32_t _sdata; //start of data in SRAM
extern uint32_t _edata; // end of data in SRAM
extern uint32_t _sbss; // start of .bss in SRAM
extern uint32_t _ebss; // end of .bss in SRAM

/*Forward declaration of handlers*/
void Reset_Handler(void);
void Default_Handler(void);

/*vector table placed in vectors section*/
__attribute__ ((section("vectors")))
void (* const vector_table[])(void) = {
    (void (*)(void))(&_estack), //initial stack pointer
    Reset_Handler,
    Default_Handler
};

void Reset_Handler(void) {
    uint32_t *src, *dst;
    /* copy data section from flash to RAM*/
    src = &_sidata;
    for (dst = &_sdata; dst < &_edata;)
        *dst++ =*src++;
    for (dst = &_sbss; dst < &_ebss;)
        *dst++ = 0;
    //call main
    extern int main(void);
    main();
    while(1);
}

void Default_Handler(void) {
    while(1);
}
