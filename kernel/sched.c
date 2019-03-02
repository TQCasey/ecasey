#include <ecasey/pcb.h>
#include <2440/2440addr.h>
#include <2440/drvs.h>
#include <ecasey/kernel.h>
#include <string.h>
#include <sys/types.h>

extern u32 NR_TASKS;
extern TASK     *current_p;
extern TASK*    task_st[];
extern u32 __ticks;

extern i32 lcd_gotoxy (i32 x,i32 y);
extern void change_ttb_base (u32 ttb_base);

void switch_to (TASK *to)
{
    if (to == current_p) return ;

    current_p = to;
    current_p->state = TS_RUN;
    change_ttb_base (v2p(current_p->ttb_base));
}
/* 
 * O (NR_TASKS) schedule () 
 * if little NR_TASKS come,this func runs very fast
 * if more  NR_TASKS ,some tough
 * if huge NR_TASKS come ,it may be very slow !!! 
 */
i32 schedule (void)
{
    i32 i = 0,j = 0 ,maxi = 0;

    for ( i = 1; i < MAX_TASKS && j < NR_TASKS; i ++) {
        if (!task_st[i] || TS_RUN != task_st[i]->state)
            continue;
        j ++ ;
        if (task_st[i]->tick >= task_st[maxi]->tick)
            maxi = i;
    }
    if (!task_st[maxi]->tick) {  
        for (i = 0,j = 0; i < MAX_TASKS && j < NR_TASKS; i ++) {
            if (!task_st[i])
                continue;
            j ++;
            task_st [i]->tick = task_st[i]->prio;
        }
        maxi = 0;
    }
    switch_to (task_st[maxi]);
    return (maxi);
}

void sleep_task (TASK *task)
{
    if (!task || TS_RUN != task->state )  return ;
    task->state = TS_HANG;
    schedule ();
}

void wake_task (TASK *task)
{
    if (!task || TS_RUN == task->state )  return ;
    task->state = TS_RUN;
    schedule ();
}

void __do_timer0 (void)
{
    if (current_p->tick > 0)
        current_p->tick --;

    __ticks ++;
    schedule ();
    /* DO NOT TOUCH IT till you know what u gonna do */
	rSRCPND |= (1<<10);
	rINTPND |= (1<<10);	
}

