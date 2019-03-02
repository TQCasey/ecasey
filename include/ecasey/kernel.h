/* 
 * FOR TQ2440 KERNEL.H
 * 2013 - 4 - 12 
 * BY CASEY
 *
 */

#ifndef __KERNEL_H__
#define __KERNEL_H__
#include <sys/types.h>

#define PCLK            (50000000)  /* 50 M default */
#define BAUD_RATE	    (115200)
#define FIN             (12000000)  /* Fin 12Mhz */


#define __sti()       \
{\
    __asm__  __volatile__ (\
        "mrs    r0,cpsr\n"\
        "bic    r0,r0,#1<<7\n"\
        "msr    cpsr,r0\n"\
        ::: "r0"\
    );\
}

#define __cli()       \
{\
    __asm__  __volatile__ (\
        "mrs    r0,cpsr\n"\
        "orr    r0,r0,#1<<7\n"\
        "msr    cpsr,r0\n"\
        ::: "r0"\
    );\
}


#define printk      uart_printf
#define xprintk     lcd_printf
extern u32          k_reenter;
extern u32          __ticks;
#define CUR_TICKS   __ticks

#endif
