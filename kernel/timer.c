#include <2440/2440addr.h>
#include <2440/drvs.h>
#include <sys/types.h>

extern void __do_timer0 (void);

/* timer0 to do the schedule () */
void init_timer (i32 nr)
{	
    rTCFG0 &= ~(0xFF<<0);
    rTCFG0 |= 0xF9;			/* 250 prescaler */

    rTCFG1 &= ~(0x0F);
    rTCFG1 |= 0x03;			/* 16 division */

    rTCNTB0 = 125;		    /* (1s = /freq = (PCLK)/(((prescaler+1)*division) */
    rTCMPB0 = 0;            /* 10ms */

    rSRCPND |= (1<<10);
    rINTPND |= (1<<10);	

    pISR_TIMER0 = (u32)__do_timer0;

    rINTMSK &= ~(1<<10);	

    rTCON &= ~0x0F;			/* clear [3:0] */
    rTCON |= (1<<1);		/* set buffer & cmp */

                            /* some nop here ! */

    rTCON &= ~0x0F;			/* clear [3:0] */
    rTCON |= 0x09;			/* start timer0 */
}
