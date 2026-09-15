/* Define memory regions for Stellaris LM4F120 (Cortex-M3)*/
MEMORY
{
 FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K /*Program storage*/
 SRAM  (rwx): ORIGIN = 0X20000000, LENGTH = 32K /*Runtime Memory*/
}

SECTIONS
{
 .text : {
   KEEP(*(vectors)) /*Keep vector table at start*/
   *(.text*)       /*Program code*/
   *(.rodata*)     /*Read only data*/
 } > FLASH

 /*Initialized data: stored in FLASH, copied to SRAM at startup*/
 .data : {
   _sdata = .;
   *(.data*)
   _edata = .;
 } > SRAM AT > FLASH
 
 /*Uninitialized data: zeroed in SRAM at startup*/
 .bss : {
   _sbss = .;
   *(.bss*)
   *(COMMON)
   _ebss = .;
 } > SRAM
}
/*Stack pointer defined at the end of SRAM*/
_estack = ORIGIN(SRAM) + LENGTH(SRAM);
_sidata = LOADADDR(.data); 


