
#ifndef __MAIL_H__
#define __MAIL_H__

typedef struct _mail_st {
    union _msg {                    /* message / return */
        unsigned    umsg;
        int         ret;
    }msg;
    int             from,to;        /* from/to whom ? */
    void            *src;           /* text src*/
    unsigned        len;            /* text len */
    struct _mail_st *next;          /* next message */
    unsigned        stime;          /* send time */
}mail_st;

#define MT_SEND         0
#define MT_POST         1

#define TASK_SYS        0
#define TASK_FS         1
#define TASK_SH         2

#define TASK_ANY        (-1)
#define NO_TASK         (-1)

#define MSG_OK          (0)
#define MSG_ERR         (1)

#endif
