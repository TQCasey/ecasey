/* 
 * process Control Block header files 
 * 2013 - 4 - 22 by casey 
 */

#ifndef __PCB_H__
#define __PCB_H__

#include <ecasey/mail.h>

typedef struct s_regs_frame {
    unsigned    lr;                 /* usr lr */
    unsigned    sp;                 /* usr sp */
    unsigned    spsr;               /* spsr */
    unsigned    r0;     
    unsigned    r1;
    unsigned    r2;
    unsigned    r3;
    unsigned    r4;
    unsigned    r5;
    unsigned    r6;
    unsigned    r7;
    unsigned    r8;
    unsigned    r9;
    unsigned    r10;                /* r10 */
    unsigned    fp;                 /* r11 */
    unsigned    ip;                 /* r12 */
    unsigned    pc;                 /* r14/r15 (img entry) */
}REGS_FRAME;

#define SEND_BUF_SIZE   (1024)

/* process control block */
typedef struct pcb_st {
    REGS_FRAME      regs;
    unsigned        ttb_base;       /* ttb base */
    unsigned        sigmap;         /* signal bit map */
    int             pid;            /* process id */
    char            prio;           /* priority */
    char            tick;           /* round life tick */
    char            state;          /* task state */
    struct pcb_st   *head,*tail;    /* send mail queue */
    struct pcb_st   *next;          /* queue link */
    mail_st         mail;           /* current send mai */
}PCB,TASK;

#define PL_MAX          5
#define TS_MASK         7

#define TS_RUN          0
#define TS_PND          1
#define TS_SEND         2
#define TS_RECV         3
#define TS_HANG         4

#define  MAX_TASKS      (256)

extern TASK* task_st[];

#endif 
