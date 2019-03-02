#include <stdarg.h>
#include <string.h>
#include <ecasey/mail.h>
#include <ecasey/sysno.h>
#include <sys/types.h>
#include <ecasey/rtc.h>
#include <sys/types.h>
#include <sys/types.h>

extern i32 sendm (i32 to,u32 umsg,void *src,u32 len);
LOCAL i32 _get_ticks (void)
{
    return (sendm (0,SYS_GET_TICK,"",0));
}

LOCAL i32 _uart_write (void *addr,u32 size)
{
    if (!addr || !size) 
        return (0);
    return (sendm (0,SYS_UART_WRITE,addr,size));
}

LOCAL i32 _lcd_write (void *addr,u32 size)
{
    if (!addr || !size) 
        return (0);
    return (sendm (0,SYS_LCD_WRITE,addr,size));
}

i32 pause (void)
{
    return (sendm (0,SYS_PAUSE,"",0));
}

i32 printf (RO i8 *fmt,...) 
{
    va_list ap = (void*)0;
    i32 i = 0 ;
    i8 string[1024] = {0};
    va_start(ap,fmt);
    i = vsprintf(string,fmt,ap);
    va_end (ap);
    _uart_write ( (void*)string,i);
    return (i);
}

i32 xprintf (RO i8 *fmt,...) 
{
    va_list ap = (void*)0;
    i32 i = 0 ;
    i8 string[1024] = {0};
    va_start(ap,fmt);
    i = vsprintf(string,fmt,ap);
    va_end (ap);
    _lcd_write ( (void*)string,i);
    return (i);
}

void delay_ms (i32 ms)
{
    i32 t = _get_ticks ();
    while (((_get_ticks () - t) * 10)  < ms);
}

void delay_sec (i32 sec)
{
    i32 t = _get_ticks ();
    while (((_get_ticks () - t) / 100) < sec);
}

i32 gettime (systm_st *t) 
{
    if (!t) 
        return (0);
    return (sendm (0,SYS_GETTIME,t,sizeof(systm_st)));
}

i32 settime (systm_st *t)
{
    if (!t) 
        return (0);
    return (sendm (0,SYS_SETTIME,t,sizeof(systm_st)));
}

u32 getadc (u32 ch)
{
    return (sendm (0,SYS_GETADC,(void*)ch,0));
}

u32 getpos (u16 *x,u16 *y)
{
    i8 buf[4];
    i32 ret = sendm (0,SYS_GETPOS,buf,4);
    *x = getw(buf); 
    *y = getw(buf+2);
    return (ret);
}

u32 nand_rw (u8 rw,u32 blkno,void *buf)
{
    u32 p [3] ,ret;

    p [0] = (u32)rw;
    p [1] = (u32)blkno;
    p [2] = (u32)buf;

    ret = sendm (0,SYS_RW_NAND,(void*)p,sizeof(p));
    return (ret);
}

u32 getchs (i8 *buf,u32 size)
{
    if (!buf || !size) 
        return (0);
    return (sendm (0,SYS_GETCHS,buf,size));
}

u32 sync_tl (void)
{
    return (sendm (0,SYS_SYNC_TL,"",0));
}
