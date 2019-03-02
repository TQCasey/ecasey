#include <2440/2440addr.h>
#include <2440/drvs.h>
#include <ecasey/mail.h>
#include <ecasey/kernel.h>
#include <sys/types.h>

u32 sys_adc (mail_st *m)
{
    if (!m) 
        return (0);
    u32 ch = (u32)(m->src);

    rADCCON = (1<<14) | (PRSCVL << 6) | ((ch & 7) << 3);
    rADCTSC &= ~(1<<2);             /* normal adc */

    rADCCON |= (1<<0);              /* start */
    while (rADCCON & 0x01);         /* wait to start */

    while (!(rADCCON & (1<<15)));   /* end adc */

    return (rADCDAT0 & 0x3FF);      /* READ */
}
