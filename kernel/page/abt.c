#include <2440/2440addr.h>
#include <2440/drvs.h>
#include <2440/ports.h>
#include <2440/cache.h>
#include <assert.h>

LOCAL 
u32 get_fsr_type (void)          
{
    u32  fsr_type;
    __asm__  __VAR__ (
        "mrc p15,0,%0,c5,c0,0\n"
        :"=r"(fsr_type)
    );
    return (fsr_type);
}

LOCAL 
u32 get_fsr_addr (void)
{
    u32  fsr_addr;
    __asm__  __VAR__ (
        "mrc p15,0,%0,c6,c0,0\n"
        :"=r"(fsr_addr)
    );
    return (fsr_addr);
}

typedef i32 (*FSR_PROC) (RO i8 *desc,u32 type,u32 addr);

struct fsr_struct {
    i32         fsr_idx;
    FSR_PROC    fsr_proc;
    i8*       desc;
};

extern i32 do_vect_fault    (RO i8 *desc,u32 type,u32 addr);
extern i32 do_align_fault   (RO i8 *desc,u32 type,u32 addr);
extern i32 do_term_fault    (RO i8 *desc,u32 type,u32 addr);
extern i32 do_ext_abt       (RO i8 *desc,u32 type,u32 addr);
extern i32 do_sect_fault    (RO i8 *desc,u32 type,u32 addr);
extern i32 do_page_fault    (RO i8 *desc,u32 type,u32 addr);
extern i32 do_trans_fault   (RO i8 *desc,u32 type,u32 vaddr);
extern i32 do_perm_fault    (RO i8 *desc,u32 type,u32 vaddr);

LOCAL struct fsr_struct fsr_st [] = {
    {0x00,do_vect_fault, "vector exception"},
    {0x01,do_align_fault,"alignment fault" },
    {0x02,do_term_fault, "terminal exception"},
    {0x03,do_align_fault,"alignment fault"},
    {0x04,do_ext_abt,    "section external abort on line-fetch"},
    {0x05,do_sect_fault, "section translation fault"},
    {0x06,do_ext_abt,    "page external abort on line-fetch"},
    {0x07,do_page_fault, "page translation fault"},
    {0x08,do_ext_abt,    "section external abort on non-linefetch"},
    {0x09,do_sect_fault, "section domain fault"},
    {0x0A,do_ext_abt,    "page external abort on non-translation "},    
    {0x0B,do_page_fault, "page domain fault "},
    {0x0C,do_trans_fault,"level 1 translation fault"},
    {0x0D,do_perm_fault, "section permission fault"},
    {0x0E,do_trans_fault,"l2 translation fault"},
    {0x0F,do_perm_fault, "page permission fault"}
};

/* 
 * data abort can not be scheduled 
 */

void do_data_abt (void)
{
    u32 addr = get_fsr_addr ();
    u32 fsr = get_fsr_type ();
    u32 domain,type;

    type = fsr & 0xF;
    domain = (fsr>>4)& 0x0F;
    (*fsr_st [type].fsr_proc) (fsr_st[type].desc,type,addr);
}


