#include <2440/2440addr.h>
#include <2440/drvs.h>
#include <ecasey/kernel.h>
#include <ecasey/pcb.h>
#include <ecasey/mail.h>
#include <ecasey/sysno.h>
#include <sys/types.h>

typedef u32 (*SYS_CALL) (u32,u32,u32,u32);
LOCAL SYS_CALL sys_call_tbl[MAX_SYSCALL] = {0};
extern TASK *current_p;

/*
 * send mail takes 0x80 syscall no 
 */
void do_sys_call (u32 sys_no)
{
    SYS_CALL sys_call = (SYS_CALL)sys_call_tbl[sys_no];
    u32 a,b,c,d;

    a = current_p->regs.r0;
    b = current_p->regs.r1;
    c = current_p->regs.r2;
    d = current_p->regs.r3;

    (*sys_call) (a,b,c,d);       /* just do it ! */
}

#define set_sys_call(nr,call)  sys_call_tbl[nr] = call

u32 __ticks = 0;

extern i32 lcd_putch (u32 c,u32 bk_c,u32 st,i8 ch);

i32 sys_lcd_write (mail_st *m)
{
    u8 *p = (u8*)m->src;
    i32 size = m->len;

    while (size --) 
        lcd_putch (0,0xFFFF,0, *p++);
    return (size);
}

i32 sys_uart_write (mail_st *m)
{
    u8 *p = (u8*)m->src;
    i32 size = m->len;

    while (size --) 
        uart_sendc (*p++);
    return (size);
}

extern void sleep_task (TASK *task);

i32 sys_pause (void)
{
    sleep_task (current_p);
    return (0);
}

extern mail_st* sys_sendm (mail_st *m);
extern mail_st* sys_postm (mail_st *m);
extern mail_st* sys_recvm (mail_st *m);
extern mail_st* sys_replym(mail_st *m);

LOCAL 
void bad_swi (void)
{
    printk ( "$ > bad swi nr !\n" );
}

void setup_syscalls (void)
{
    i32 i = 0 ;

    for (i = 0 ; i < MAX_SYSCALL ; i ++ )
        sys_call_tbl [i] = (SYS_CALL)bad_swi;

    set_sys_call (0x03,(SYS_CALL)sys_pause);
    set_sys_call (0x80,(SYS_CALL)sys_sendm);
    set_sys_call (0x81,(SYS_CALL)sys_postm);
    set_sys_call (0x82,(SYS_CALL)sys_recvm);
    set_sys_call (0x83,(SYS_CALL)sys_replym);
}
extern i32 sys_gettime (mail_st *m);
extern i32 sys_settime (mail_st *m);
extern u32 sys_adc (mail_st *m);
extern u32 sys_getpos (mail_st *m);
extern i32 sys_rw_nand (mail_st *m);
extern i32 sys_getchs (mail_st *m);
extern i32 sys_sync_tl (void);

i32 do_sys_msg (mail_st *m)
{
    if (!m) 
        return (-1);
    switch (m->msg.umsg)
    {
        case SYS_GET_TICK:
            return (__ticks);
        case SYS_UART_WRITE:
            return (sys_uart_write (m));
        case SYS_LCD_WRITE:
            return (sys_lcd_write(m));
        case SYS_PAUSE:
            return (sys_pause ());
        case SYS_GETTIME:
            return (sys_gettime (m));
        case SYS_SETTIME:
            return (sys_settime (m));
        case SYS_GETADC:
            return (sys_adc (m));
        case SYS_GETPOS:
            return (sys_getpos(m));
        case SYS_RW_NAND:
            return (sys_rw_nand (m));
        case SYS_GETCHS:
            return (sys_getchs (m));
        case SYS_SYNC_TL:
            return (sys_sync_tl ());
    }
    return (0);
}
