#include <ecasey/mail.h>
#include <string.h>
#include <sys/types.h>

/* sendm() sends mail to dest process with a buffer .
 * In ecasey everything is text source except message,
 * when sendm () returns,the returned text source will
 * will be in sendbuff,so send buffer better be big...
 *                              -- casey    2013-7-3
 */
i32 sendm (i32 to,u32 umsg,void *src,u32 len)
{
    mail_st m;

    m.to       = to;
    m.msg.umsg = umsg;
    m.src      = src;
    m.len      = len;

    __asm__ __volatile__ ( "mov     r0,%0\n"
                           "swi     #0x80\n"
                           :                    /* no output */
                           :"r"(&m)
                           :"r0" 
    );
    return (m.msg.ret);
}

i32 postm (i32 to,u32 umsg,void *src,u32 len)
{
    mail_st m;

    m.to       = to;
    m.msg.umsg = umsg;
    m.src      = src;
    m.len      = len;

    __asm__ __volatile__ ( "mov     r0,%0\n"
                           "swi     #0x81\n"
                           :                    /* no outpput */
                           :"r"(&m)
                           :"r0"         
    );
    return (m.msg.ret);
}

i32 recvm (i32 from,i32 *sender,u32 *msg,void *buf,u32 size)
{
    if (!buf || !size) return (0);

    mail_st m;

    m.from = from;
    m.src  = buf;
    m.len  = size;

    __asm__ __volatile__ ( "mov     r0,%0\n"
                           "swi     #0x82\n"
                           :                    /* no outpput */
                           :"r"(&m)
                           :"r0"         
    );
    *sender = m.from;
    *msg = m.msg.umsg;
    return (m.len);                             /* return the bytes already sent */
}

i32 replym (i32 to,u32 umsg,void *src,u32 len)
{
    mail_st m;

    m.to        = to;
    m.msg.umsg  = umsg;
    m.src       = src;
    m.len       = len;

    __asm__ __volatile__ ( "mov     r0,%0\n"
                           "swi     #0x83\n"
                           :                    /* no outpput */
                           :"r"(&m)
                           :"r0"         
    );
    return (m.msg.ret);
}
