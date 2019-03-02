OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

SECTIONS
{
	. = 0x60000000;
    . = ALIGN(4);
    .text :
    {
        *(.text)
    }

    . = ALIGN(4);
    .rodata :
    {
        *(.rodata)
    }

    . = ALIGN(4);
    .data :
    {
        *(.data)
    }

    . = ALIGN(4);
    _bss_start  = . ;
    .bss (NOLOAD):
    {
        *(.bss) *(COMMON)
    }
    _bss_end = . ;
}
