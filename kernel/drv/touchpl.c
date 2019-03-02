#include <2440/2440addr.h>
#include <2440/drvs.h>
#include <sys/types.h>
#include <ecasey/kernel.h>
#include <ecasey/mail.h>
#include <sys/types.h>

/* ADC freq < PCLK/5 */ 
LOCAL u16 x,y;


#define wait_down() {rADCTSC = 0xD3 | (0<<8);}
#define wait_up()   {rADCTSC = 0xD3 | (1<<8);}
#define auto_xy()   {rADCTSC = (1<<3) | (1<<2);}

LOCAL void touch_prc (void)
{
    if (rADCDAT0 & (1<<15)) { 
        wait_down ();                       /* wait again */
    } else {
        auto_xy ();                         /* auto xy mode */
        rADCCON |= 0x01;                    /* start adc */
    }
}

LOCAL void adc_prc (void)
{
    x = rADCDAT0 & 0x3FF;
    y = rADCDAT1 & 0x3FF;
    wait_up ();
}

LOCAL 
void adc_touch_prc (void)
{
    if (rSUBSRCPND & (1<<9)) {
        touch_prc ();
        rSUBSRCPND |= (1<<9);
    }
    if (rSUBSRCPND & (1<<10)){
        adc_prc ();
        rSUBSRCPND |= (1<<10);
    }
    rSRCPND     |= (1<<31);
    rINTPND     |= (1<<31);                 /* happy... */
}

/* ADC freq: 2Mhz,time to convert : 2.5us */
void touchpl_init (void) 
{
    /* ADC controller configure */
    rADCCON = (1<<14)|(PRSCVL<<6); 
    rADCDLY = 50000;                        /* clean up the startup ADC data */
    wait_down ();                           /* wait for intr */

    rSRCPND |= (1<<31);
    rSUBSRCPND |= (1<<9);
    rSUBSRCPND |= (1<<10);
    rINTPND |= (1<<31);

    pISR_ADC = (u32)&adc_touch_prc;

    rINTSUBMSK &= ~(1<<9);                  /* tc */
    rINTSUBMSK &= ~(1<<10);                 /* adc */
    rINTMSK &= ~(1<<31);                    /* go go go */
}

u32 sys_getpos (mail_st *m)
{
    if (!m) return (0);
   *(u16 *)(m->src)      = x;
   *(u16 *)(m->src+2)    = y;
   return (1);
}
