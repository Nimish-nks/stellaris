MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x00040000 /* 256KB */
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00008000 /* 32KB */
}

SECTIONS
{
    .text :
    {
        _text = .;
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)
        _etext = .;
    } > FLASH

    .data :
    {
        _data = .;
        *(.data*)
        _edata = .;
    } > SRAM AT > FLASH

    .bss :
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > SRAM
}
