#include <2440/2440addr.h>
#include <2440/drvs.h>
#include <2440/ports.h>
#include <sys/types.h>

LOCAL u32 *irq_table = (u32*)(0x00);

void nop (void)
{
    ;
}

extern void _do_undef_instruction (void);
extern void _do_sys_call (void);
extern void _do_prefetch_abt (void);
extern void _do_data_abt (void);
extern void _do_reserved (void);
extern void _do_irq (void);
extern void _do_fiq (void);

void init_intr_vector (void)
{
    i32 i = 1 ;

    irq_table = (u32*)(_ISR_STARTADDRESS);

    irq_table [1] = (u32)_do_undef_instruction;
    irq_table [2] = (u32)_do_sys_call;
    irq_table [3] = (u32)_do_prefetch_abt;
    irq_table [4] = (u32)_do_data_abt;
    irq_table [5] = (u32)_do_reserved;
    irq_table [6] = (u32)_do_irq;
    irq_table [7] = (u32)_do_fiq;

    for ( i = 8 ;i < 256 ; i ++) 
        irq_table [i] = (u32) (&nop);
}

void set_irq (i32 irq_nr,u32 func)
{
    if (irq_nr >= 0 && irq_nr < 256) 
        irq_table [irq_nr] = func;
}

void unset_irq (i32 irq_nr)
{
    if (irq_nr >= 0 && irq_nr < 256) 
        irq_table [irq_nr] = (u32)&nop ;
}

typedef i32 (*IRQ_FUNC) (void);

i32 do_irq (void)
{
    i32 irq_nr = rINTOFFSET;
    irq_nr += 8;                        /* skip exception index */       
    u32 entry = *((u32*)((_ISR_STARTADDRESS + (irq_nr << 2))));
    IRQ_FUNC func = (IRQ_FUNC)entry;    /* get func addr from irq_table */
    return (func ());                   /* just do it */
}
