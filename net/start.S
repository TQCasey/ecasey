    .global  _start
    .extern main
    .code 32
    .text
_start:
    bl      clear_bss
    bl      main
    b       .

clear_bss:
	ldr	    r0, bss_start	            /* clear bss start */	
	ldr	    r1, bss_end		            /* bss end */
	mov	    r2, #0x00                   /* set 0x00 */

clr:str	    r2, [r0]		            /* clear loop */
	add	    r0, r0, #4
	cmp 	r0, r1
	ble	    clr                         /* -- */
    mov     pc,lr                       /* ret */


bss_start:      .word   _bss_start
bss_end:        .word   _bss_end
