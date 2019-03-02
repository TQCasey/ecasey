#include <2440/ports.h>
    .global _do_data_abt
    .extern do_data_abt,iret
    .text   
    .code   32

_do_data_abt:
    ldr     sp,=current_p               /* switch to pcb */
    ldr     sp,[sp]                     /* get pcb ptr */
    add     sp,sp,#68                   /* point to top of stack */
    sub     lr,lr,#0x08                 /* return addr */
    stmdb   sp!,{r0-r12,lr}             /* prepare to return */ 
    mrs     r1,spsr                     /* r1 <- spsr_usr */
    mrs     r2,cpsr                     /* r2 <- cpsr */
    push    {r1}                        /* save spsr_usr */
    msr     cpsr_c,#SYS_MODE|NO_INTR    /* switch to sys mode */
    mov     r3,lr                       /* r3 <- lr_usr */
    mov     r4,sp                       /* r4 <- sp_usr */
    msr     cpsr,r2                     /* switch back */
    push    {r3-r4}                     /* save sp_usr,lr_usr */
    msr     cpsr_c,#SVC_MODE|NO_INTR    /* switch back to SVC mode */
    ldr     sp,=KERNEL_STACK            /* switch to SVC stack */
    push    {r0-r3}                     /* push r0 - r3 */
    ldr     lr,=iret                    /* set return address */
    ldr     pc,=do_data_abt             /* call do_data_abt */
    b       .                           /* should never reach here ! */
