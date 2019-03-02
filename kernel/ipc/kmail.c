#include <2440/2440addr.h>
#include <2440/drvs.h>
#include <ecasey/mail.h>
#include <ecasey/kernel.h>
#include <ecasey/pcb.h>
#include <sys/types.h>

#define READ    1
#define WRITE   0

/* this is simple mail communications code between processes
 * which is copied && simplified from caseyOS (x86) 
 *                                          -- casey
 */
extern TASK *current_p;

LOCAL i32 try_to_send (mail_st *m)
{
    mail_st *next = &(task_st[m->to]->mail);
    while (next->to != NO_TASK) {
        if (m == next) return (1);
        next = &((task_st [next->to])->mail);
    }
    return (0);
}

void dumpm (mail_st *m)
{
    printk ("from pid = %d \n"
            "to   pid = %d \n"
            "umsg     = 0x%0x\n"
            "src      = 0x%0x\n"
            "len      = %d\n"
            ,m->from,m->to,m->msg.umsg,m->src,m->len
           );
}

extern i32 do_sys_msg (mail_st *m);
extern void switch_to (TASK *to);

i32 sys_sendm (mail_st *m)
{
    if (!m) return (0);
    m->from = current_p->pid;
    if (m->to < 0 || m->to > MAX_TASKS 
    || m->from == m->to || !m->from || !task_st[m->to])
        return (0);
    if (!m->to)                                 /* system call */
        return (m->msg.ret = do_sys_msg (m));
    if (try_to_send (m)) {                      /* loop mail detected ! */
        printk ( "ipc: loop mail send denied !\n" );
        return (0);
    } else 
        current_p->mail = *m;
    /* queue the sendm process */
    PCB *p = task_st[m->to];
    if (!p->head) {                             /* null queue */
        p->tail = p->head = current_p;
        p->tail->next = (void*)0;
    }
    else { 
        p->tail->next = current_p;
        p->tail = current_p;
    }
    current_p->state = TS_SEND;                 /* pend */
    switch_to (task_st[m->to]);                 /* switch to dest process */
    m->msg.ret  = 1;
    return (1);
}

i32 physrw (i32 rw,u8 *addr1,i32 pid,
            u8 *addr2,i32 size) 
{
    if (pid < 0 || pid > MAX_TASKS 
    || !task_st[pid] || size < 0 || !addr1 || !addr2) 
        return (0);

    VAR u32 *page_dir,*page_tbl,*page,vaddr = (u32)addr2;
    i32 l1_idx,l2_idx,offset,cnt = 0,chars = 0;
    u8 *p = (void*)0;

    page_dir = (VAR u32*)(task_st[pid]->ttb_base);
    offset = (vaddr & 0xFFF);
    while (size > 0) {
        l1_idx = vaddr >> 20;
        l2_idx = (vaddr >> 12) & 0xFF;

        chars = 0x1000 - offset;
        if (chars > size) 
            chars = size;
        size -= chars ;
        cnt += chars;

        if (!(page_dir[l1_idx] & ~0x3FF))       /* empty page dir */
            break;
        else {
            page_tbl = (VAR u32 *)(p2v(page_dir [l1_idx] & ~0x3FF));
            if (!(page_tbl[l2_idx] & ~0xFFF)) break;
            else {                              /* page tbl */
                page = (VAR u32 *)(p2v(page_tbl [l2_idx] & ~0xFFF));
                p = (u8*)page + offset;
                if (rw == READ)
                    while (chars -- > 0) *addr1 ++ = *p ++;
                else
                    while (chars -- > 0) *p ++ = *addr1 ++;
            }
        }
        offset = 0;
        vaddr += 0x1000;
    }
    return (cnt);
}

extern i32 schedule (void);

/* recv mail from sending queue */
i32  sys_recvm (mail_st *m)
{
    if (!m) return (0);
    i32 from = m->from;
    if (from == TASK_ANY) {                     /* recv any mail which sent by others */
        TASK *head = current_p->head;
        if (!head) {                            /* queue empty */
            current_p->state = TS_RECV;
            current_p->regs.pc -= 4;            /* do it again */
            schedule ();                        /* schedule it */
            return (0);
        }
        /* yes ,we have got one ! */
        if (!m->src || !m->len) {
            printk ( "ipc : recvm buffer is empty !\n" );
            return (0);
        }
        /* copy text src from sender */
        m->len = physrw (READ,m->src,head->pid,
                        head->mail.src,head->mail.len);
        m->msg.umsg = head->mail.msg.umsg;
        m->from = head->pid;                    /* from pid */
        return (1);
    }
    printk ( "unique recvm is not implemted !\n" );
    return (0);
    /* unique recv mail */
    if (from <= 0 || from > MAX_TASKS || !task_st[from])
        return (0);
    return (0);
}

i32 sys_replym (mail_st *m)
{
    if (!m) return (0);
    m->from = current_p->pid;
    if (m->to < 0 || m->to > MAX_TASKS 
    || m->from == m->to || !m->from || !task_st[m->to])
        return (0);
    TASK *p = current_p->head;
    if (!p)                                     /* send queue empty ! */
        return (0);
    else {
        if (!m->src || !m->len) {
            printk ( "ipc : replym buffer is empty !\n" );
            return (0);
        }
        p->mail.msg.ret = m->msg.umsg;
        /* copy reply mail & text source */
        m->msg.ret = physrw (WRITE,m->src,p->pid,
                    p->mail.src,p->mail.len); 
        /* pick the sendm process down from sendque */
        p->state     = TS_RUN;
        p->mail.to   = NO_TASK;
        current_p->head = p->next;

        if (p->next == (void*)0) {              /* empty queue */
            current_p->head = current_p->tail = (void*)0;
            current_p->next = (void*)0;
        }
    }
    return (0);
}

/* posted mail group API */
i32 sys_postm (mail_st *m)
{
    return (1);
}

i32 sys_pickm (mail_st *m) 
{
    return (1);
}
