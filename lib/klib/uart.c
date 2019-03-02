/* 
 * FOR TQ2440 UART 
 * BY CASEY 
 * 2013 - 3 - 28 
 */
#include <ecasey/pcb.h>
#include <2440/2440addr.h>
#include <2440/drvs.h>
#include <ecasey/kernel.h>
#include <string.h>
#include <sys/types.h>

LOCAL i32  uart_no = 0;                /* uart 0 is used by default */
/*
 * uart 0,1,2 can be used at the same time 
 * but each configuration can be different !!
 */

void use_uart (i32 uart)
{
    if (uart < 0 || uart > 3)   return ;
    uart_no = uart;
}

void uart_conf (i32 uart,u32 pclk,u32 baud)
{
    if (uart < 0 || uart > 3)   return ;
    u32 baud_rate;

    if (pclk == 0)  pclk = PCLK;
    if (baud == 0)  baud = BAUD_RATE;

    uart_no     = uart ;
    baud_rate   = (u32) (PCLK / (BAUD_RATE << 4)) - 1;

    if (uart_no == 0) {
        rGPHCON &= ~((3<<4)|(3<<6));    
        rGPHCON |= ((2<<4)|(2<<6));     /* GPH2,GPH3 */
        rGPHUP |= 0x0C;                 /* pull up disable */
        rULCON0	= 0x03;                 /* normal, 8 datab, 1 stopb, no chkb */
        rUCON0	= 0x05;                 /* 44 div ,PCLK && polling recv & send */
        rUFCON0 = 0x00;	                /* FIFO :no used  */
        rUMCON0	= 0x00;                 /* MODEM : not used */
        rUBRDIV0 = baud_rate;           /* BAUD RATE */
    } else if (uart_no == 1) {

    } else {

    }
}

u8 uart_sendc (u8 ch)
{
    if (uart_no == 0) {
        if (ch == '\n') {
            while (!(rUTRSTAT0 & 0x02));/* send buf empty */
            rUTXH0 = '\r';              /* send a '\r' */
        }
        while ( !(rUTRSTAT0 & 0x02) );	/* send buf empty */
        rUTXH0 = ch;
    } else if (uart_no == 1) {
        if (ch == '\n') {
            while (!(rUTRSTAT1 & 0x02));/* send buf empty */
            rUTXH1 = '\r';              /* send a '\r' */
        }
        while ( !(rUTRSTAT1 & 0x02) );	/* send buf empty */
        rUTXH1 = ch;
    } else {
        if (ch == '\n') {
            while (!(rUTRSTAT2 & 0x02));/* send buf empty */
            rUTXH2 = '\r';              /* send a '\r' */
        }
        while ( !(rUTRSTAT2 & 0x02) );	/* send buf empty */
        rUTXH2 = ch;
    }
    return (0xFF);
}

u8 uart_recvc (void)
{
    if (uart_no == 0) {
	    while (!(rUTRSTAT0 & 0x01));
	    return (0xFF & rURXH0);
    } else if (uart_no == 1) {
	    while (!(rUTRSTAT1 & 0x01));
	    return (0xFF & rURXH1);
    } else {
	    while (!(rUTRSTAT2 & 0x01));
	    return (0xFF & rURXH2);
    }
    return (0xFF);
}

RO i8* uart_sends (RO i8 *s)
{
    i8 *p = (i8*)s;

	while (*p)  uart_sendc (*p++);

    return (s);
}

i32 uart_printf(RO i8 *fmt,...)
{
    va_list ap;
    i32 i = 0 ;
    i8 string[1024+1] = {0};

    va_start(ap,fmt);
    i = vsprintf(string,fmt,ap);
    uart_sends(string);
    va_end(ap);

    return (i);
}

void uart_intr_conf (u32 prc)
{
	rSRCPND	|= (1<<28);
	rSUBSRCPND |= (1<<0);
	rINTPND	|= (1<<28);
	
	pISR_UART0	= (u32)prc;
	
	rINTSUBMSK &= ~(1<<0);
	rINTMSK &= ~(1<<28);
}
