#include <2440/ports.h>
    .global _start

    .global _do_undef_instruction,_do_sys_call,_do_prefetch_abt
    .global _do_reserved,_do_irq,_do_fiq

    .global k_reenter,restart,iret

    .extern kernel_start,current_p
    .text 
    .code   32
_start:
    bl      clear_bss                   /* clear bss */
    ldr     sp,=KERNEL_STACK            /* set new stack for kernel (svc mode)*/
    ldr     pc,=kernel_start            /* just enter kernel */
    b       .                           /* halt here ! */


clear_bss:
	ldr	    r0, bss_start	            /* clear bss start */	
	ldr	    r1, bss_end		            /* bss end */
	mov	    r2, #0x00                   /* set 0x00 */

clr:str	    r2, [r0]		            /* clear loop */
	add	    r0, r0, #4
	cmp 	r0, r1
	ble	    clr                         /* -- */
    mov     pc,lr                       /* ret */

.macro      SAVE_CONTEXT      SWI_F
    ldr     sp,=current_p               /* switch to pcb */
    ldr     sp,[sp]                     /* get pcb ptr */
    add     sp,sp,#68                   /* point to top of stack */
.if (\SWI_F == 0)                       /* if it is not swi macro */
    sub     lr,lr,#0x04                 /* return addr */
.endif                                  /* lr -= 4 if not swi */
    stmdb   sp!,{r0-r12,lr}             /* prepare to return */ 
    mrs     r1,spsr                     /* r1 <- spsr_usr */
    mrs     r2,cpsr                     /* r2 <- cpsr */
    push    {r1}                        /* save spsr_usr */
    msr     cpsr_c,#SYS_MODE|NO_INTR    /* switch to sys mode */
    mov     r3,lr                       /* r3 <- lr_usr */
    mov     r4,sp                       /* r4 <- sp_usr */
    msr     cpsr,r2                     /* switch back */
    push    {r3-r4}                     /* save sp_usr,lr_usr */
    mov     r0,lr                       /* r0 <-- lr */
    msr     cpsr_c,#SVC_MODE|NO_INTR    /* switch back to SVC mode */
    ldr     sp,=KERNEL_STACK            /* switch to SVC stack */
    push    {r0-r3}                     /* save r0 -r3 */
    ldr     lr,=iret                    /* set return address */
.endm

.macro      SET_ISR_PROC     ISR_PROC   
    ldr     pc,=\ISR_PROC               /* jmp ISR */
.endm


/* so let's do system call */           /* YES I AM SWI */
_do_sys_call:                           /* system call */
    SAVE_CONTEXT    1                   /* lr is in r0 */
    ldr     r0,[r0,#-4]                 /* get swi address */
    bic     r0,r0,#0xFF000000           /* clear [31:24] */
    bl      do_sys_call                 /* call do_sys_call (nr) */


iret:
    pop     {r0-r3}                     /* pop  r0 - r3 */
restart:                                /* restore context */
    ldr     sp,=current_p               /* get pcb */
    ldr     sp,[sp]                     /* adjust back to pcb */
re_restart:                             /* ret from kernel */
    pop     {r3-r4}                     /* r3 <- lr_usr ,r4 <- sp_usr */
    mrs     r2,cpsr                     /* r2 <- cpsr */
    msr     cpsr_c,SYS_MODE|NO_INTR     /* switch to sys mode*/
    mov     lr,r3                       /* lr_usr <- r3 */
    mov     sp,r4                       /* sp_usr <-- r4 */
    msr     cpsr,r2                     /* switch back */
    pop     {r1}                        /* r1 <- spsr_usr */
    msr     spsr,r1                     /* restore spsr */
    ldmia   sp!,{r0-r12,pc}^            /* intr ret */

_do_undef_instruction:
    SAVE_CONTEXT    0                   /* not swi */
    SET_ISR_PROC    do_undef_instruction
_do_reserved:
    SAVE_CONTEXT    0                   /* not swi */
    SET_ISR_PROC    do_reserved
_do_prefetch_abt:
    SAVE_CONTEXT    0                   /* not swi */
    SET_ISR_PROC    do_prefecth_abt
_do_fiq:
    SAVE_CONTEXT    0                   /* not swi */
    SET_ISR_PROC    do_fiq
_do_irq:
    SAVE_CONTEXT    0                   /* not swi */
    SET_ISR_PROC    do_irq

/* 
 * k_reenter to identfy the time of entering kernel space 
 * k_reenter > 0  --> reenter 
 * k_reenter = 0  --> first time 
 */

k_reenter:      .long   0
bss_start:      .word   _bss_start
bss_end:        .word   _bss_end
