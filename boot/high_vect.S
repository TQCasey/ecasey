/*
 * FOR TQ2440 Low Level Inint 
 * BY Casey 2013 - 04 - 07
 */

#include <2440/ports.h>

    .text
    .code   32
_vstart:
    b       _reset                      /* reset */
    b       _undef_intstruct            /* undefined instruction */
    b       _software_intr              /* software interrupt */
    b       _fetch_abt                  /* fetch abort */
    b       _data_abt                   /* date abort */
    b       _reserved                   /* resereved */
    b       _irq                        /* irq */
    b       _fiq                        /* fiq */
    
reset:

.macro      handle  irq_nr
    ldr     r13,=IRQ_TABLE              /* get irq_table */
    add     r13,r13,#\irq_nr            /* add offset */
    ldr     r13,[r13]                   /* load to r13 */
    mov     pc,r13                      /* jmp  to (irq_table+0x24) */
.endm

    .code 32
_reset:
    handle  0
_undef_intstruct:                       /* undefined intruction (offset = 4) */
    handle  4
_software_intr:                         /* software interrupt (offset = 8) */
    handle  8
_fetch_abt:                             /* fectch abort (offset = 12) */
    handle  12
_data_abt:                              /* date abort (offset = 16) */
    handle  16
_reserved:                              /* reserved for sys (offset = 20) */
    handle  20
_irq:                                   /* irq (offset = 24) */
    handle  24
_fiq:                                   /* fiq (offset = 28) */
    handle  28
