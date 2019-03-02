#include <2440/drvs.h>
#include <string.h>
#include <ecasey/mail.h>
#include <ecasey/sysno.h>
#include <ecasey/rtc.h>
#include <sys/types.h>

i8* get_date (i8 *buffer,u32 size)
{
    if (size < 30) return NULL;

	rRTCCON |= 0x01;
	
	sprintf ( buffer,"20%02d-%02d-%02d DAY:%d  %02d:%02d:%02d ",
					BCD2O(rBCDYEAR),BCD2O(rBCDMON),BCD2O(rBCDDATE),
					BCD2O(rBCDDAY),
					BCD2O(rBCDHOUR),BCD2O(rBCDMIN),BCD2O(rBCDSEC) );
	rRTCCON &= ~0x01;

    buffer [size-1] ='\0';
    return (buffer);
}

i32 sys_gettime (mail_st *m)
{
    if (!m || (rRTCCON & 0x01))     /* token */
        return (0);
    if (!m->src || !m->len)         /* buffer null */
        return (0);
    systm_st *p = (systm_st*)(m->src);

    p->year = BCD2O(rBCDYEAR);
    p->mon  = BCD2O(rBCDMON);
    p->date = BCD2O(rBCDDATE);
    p->day  = BCD2O(rBCDDAY);
    p->hour = BCD2O(rBCDHOUR);
    p->min  = BCD2O(rBCDMIN);
    p->sec  = BCD2O(rBCDSEC);

	rRTCCON	&= ~0x01;               /* lock again */
    return (1);
}

i32 sys_settime (mail_st *m)
{
    if (!m || (rRTCCON & 0x01))     /* token */
        return (0);
    if (!m->src || !m->len)         /* buffer null */
        return (0);
    systm_st *p = (systm_st*)(m->src);

	rBCDYEAR 	= O2BCD(p->year);
	rBCDMON		= O2BCD(p->mon);
	rBCDDATE	= O2BCD(p->date);
	rBCDDAY		= O2BCD(p->day);
	rBCDHOUR	= O2BCD(p->hour);
	rBCDMIN		= O2BCD(p->min);
	rBCDSEC		= O2BCD(p->sec);    

	rRTCCON	&= ~0x01;               /* lock again */
    return (1);
}
