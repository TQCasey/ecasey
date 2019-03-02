#include <2440/2440addr.h>
#include <2440/drvs.h>
#include <ecasey/kernel.h>
#include <string.h>
#include <ecasey/pcb.h>
#include <sys/types.h>

extern TASK *current_p ;
extern i32 panic (char *fmt,...);

void do_undef_instruction (void)
{
    u32 addr = current_p->regs.pc;
    panic ( "undefined exception : 0x%0x !\n ",addr );
}

extern void do_sys_call (void);

void do_prefecth_abt (void)
{
    u32 addr = current_p->regs.pc;
    panic ( "pre-fetch abort :"
            "pid = %d \n"
            "addr = 0x%0x \n",
            current_p->pid,
            addr );
}

void do_reserved (void)
{
    panic ( "reserved !\n" );
}

void do_fiq (void)
{
    panic ( "fiq  is not supported now !!\n" );
}
