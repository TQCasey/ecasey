 SECTIONS { 
    loader  	0x00000000 : { start.o led.o nand.o loader.o }
    kloader 	0x30010000 : AT(4096) {loaderk.o led.o nand.o ../lib/klib.a}
    vect        0xFFFF0000 : AT(0x4000) { high_vect.o }

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
 
