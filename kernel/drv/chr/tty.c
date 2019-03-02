#include <2440/drvs.h>
#include <ecasey/kernel.h>
#include <assert.h>
#include <ecasey/pcb.h>
#include <2440/cache.h>
#include <sys/types.h>
#include <string.h>
#include <ecasey/mail.h>

i32 sleep_pid = 0;

#define CHS_BUF_SIZE    1024
u8    chs_buf [CHS_BUF_SIZE];
u32   chs_cnt = 0;

extern i32 schedule (void);

extern i32 sleep_pid;
extern void sleep_task (TASK *task);
extern void wake_task (TASK *task);

extern u32 NR_TASKS;
extern TASK     *current_p;
extern TASK*    task_st[];
extern u32 __ticks;

void uart_recv (void)
{
	i8 ch;
	
	rSUBSRCPND |= (1<<0);
	rSRCPND |= (1<<28);
	rINTPND	|= (1<<28);
	
	ch = 0xFF & rURXH0;

    if (ch == '\r' || chs_cnt >= CHS_BUF_SIZE) {
        ch = '\n';
        if (sleep_pid || task_st [sleep_pid]->state == TS_HANG) {
            wake_task (task_st [sleep_pid]);
            sleep_pid = 0;
        }
    }
    if (ch == 0x08) {
        if (chs_cnt) {
            uart_sendc (0x08);
            uart_sendc (' ');
            uart_sendc (0x08);
            chs_cnt --;
            chs_buf [chs_cnt] = '\0';
        }
        return ;
    } else if (ch == 0x1b) {
        return ;
    }
    chs_buf [chs_cnt ++] = ch;
    chs_buf [chs_cnt] = '\0';
    uart_sendc (ch);
}

u32 sys_getchs (mail_st *m)
{
    if (!m || !m->len)
        return (0);
    if (!chs_cnt) {     /* no chs cnt */
        current_p->state = TS_HANG;
        current_p->regs.pc -= 4;    /* do it again */
        sleep_pid = current_p->pid;
        schedule ();
        return (0);
    }
    if (m->len > chs_cnt)
        m->len = chs_cnt - 1;
    memcpy ((void*)m->src,chs_buf,m->len);
    chs_cnt = 0;
    return (m->len);
}
