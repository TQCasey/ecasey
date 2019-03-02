#include <2440/2440addr.h>
#include <2440/drvs.h>
#include <2440/ports.h>
#include <2440/cache.h>
#include <ecasey/pcb.h>
#include <ecasey/kernel.h>
#include <sys/types.h>

extern TASK *current_p;
extern i32 panic (RO i8 *fmt,...);

i32 do_vect_fault    (RO i8 *desc,u32 type,u32 addr)
{
    panic ( "panic! %s,type %d ,addr = 0x%0x \n",desc,type,addr );
    return (0);
}

i32 do_align_fault    (RO i8 *desc,u32 type,u32 addr)
{
    panic ( "panic! %s,type %d ,addr = 0x%0x \n",desc,type,addr );
    return (0);
}

i32 do_term_fault    (RO i8 *desc,u32 type,u32 addr)
{
    panic ( "panic! %s,type %d ,addr = 0x%0x \n",desc,type,addr );

    return (0);
}

i32 do_ext_abt    (RO i8 *desc,u32 type,u32 addr)
{
    panic ( "panic! %s,type %d ,addr = 0x%0x \n",desc,type,addr );

    return (0);
}
extern u32 find_free_page (void);
extern void *wordset (void *s,u32 w,i32 szie);
/* 
 * one coarse page_dir cost just 1K ,find_free_page () returns 
 * a full 4K ,so we set 4 continuous page_dirs to take 4K page alloced
 */
extern void sync_dcaches (void);

extern TASK *current_p;
i32 do_page_fault (RO i8 *s,u32 type,u32 vaddr)
{
    if (!(VMA_AREA(vaddr)))
        panic ( "addr 0x%0x is not in VMA AREA !\n",vaddr );
    VAR u32 *page_dir = (void*)0,*page_tbl = (void*)0;
    u32 page = 0,i;
    u32 l1_idx = vaddr>>20,l2_idx = (vaddr >> 12) & 0xFF;
    page_dir = (VAR u32 *)(current_p->ttb_base);

    if (!(page_dir[l1_idx] & ~(0x3FF)) && (type == 0x0B)) {         /* page_dir empty */
        i = l1_idx & ~3;
        /* if one of 4M alignment L1 entries is empty
         * let 4-continuous entries ptr to the same 
         * page ,and then set those offset,any way
         * the 4entries must be zero,or else panic !
         */
        if ((page_dir[i+0] & ~0x3FF) || (page_dir[i+1] & ~0x3FF)
        ||  (page_dir[i+2] & ~0x3FF) || (page_dir[i+3] & ~0x3FF) )
        {
            panic ( "page dir corrupts with l1_idx %d ,l2_idx %d !\n"
                    ,l1_idx,l2_idx );
        }
        if (!(page = find_free_page ())) 
            panic ( "no more free page !\n" );                      /* alloc a page page dir*/
        wordset ((void*)page,AP_FAULT_ALL|CB|TTB1_SPG,0x1000);      /* we'd better clean it up ! */
        page = v2p(page);
        page_dir[i+0] = (page + 0x000)|DOMAIN_SYS|TTB0_COARSE;      /* the 1st 1KB */
        page_dir[i+1] = (page + 0x400)|DOMAIN_SYS|TTB0_COARSE;      /* the 2nd 1KB */
        page_dir[i+2] = (page + 0x800)|DOMAIN_SYS|TTB0_COARSE;      /* the 3rd 1KB */
        page_dir[i+3] = (page + 0xC00)|DOMAIN_SYS|TTB0_COARSE;      /* the 4th 1KB */

        if (!(page = find_free_page ())) 
            panic ( "no more free page !\n" );                      /* alloc a page page*/
        page = v2p(page);
        page_tbl = (VAR u32*)(p2v((page_dir[l1_idx] & ~(0x3FF))));
        page_tbl [l2_idx] = page|AP_RW_ALL|CB|TTB1_SPG; 

    } else if ((page_dir[l1_idx] & ~(0x3FF)) && (type == 0x0F)) {   /* page_dir empty */
        page_tbl = (VAR u32*) (p2v((page_dir[l1_idx] & ~(0x3FF))));
        if (!(page = find_free_page ())) 
            panic ( "no more free page !\n" );                      /* alloc a page page table*/
        page = v2p(page);
        page_tbl [l2_idx] = page|AP_RW_ALL|CB|TTB1_SPG; 
    } else {
        panic ( "bad bad bad ! pid :  %d ,address 0x%0x lr = 0x%0x \n"
                ,current_p->pid,vaddr,current_p->regs.lr );
    }
    /* safe is better than sorry 
     * clean the whole dcache
     */
    sync_dcaches ();                                                /* sync dcache -> external mem */
    invalidate_tlb (vaddr); 
    return (0);
}

i32 do_sect_fault (RO i8 *desc,u32 type,u32 vaddr)
{
    panic ( "panic ! %s,type = %d,addr = 0x%0x pid = %d \n",desc,type,vaddr,current_p->pid);
    return (0);
}

i32 do_trans_fault (RO i8 *desc,u32 type,u32 vaddr)
{
    panic ( "panic ! %s,type = %d,addr = 0x%0x pid = %d \n",desc,type,vaddr,current_p->pid);
    return (0);
}

i32 do_perm_fault (RO i8 *desc,u32 type,u32 vaddr)
{
    panic ( "panic ! %s,type = %d,addr = 0x%0x pid = %d \n",desc,type,vaddr,current_p->pid);
    return (0);
}
/* 
 * this func is more likey to be a duplicate of the do_page_fault () 
 * it map vaddr to actual phyiscal address ,indeed 4Kb is required !!
 * this is impelement for diffirent ttb base .
 * it map virtual address based on what the ttb_base is !
 * if the ttb base equ current ttb_base ,we need flush dcache & tlb
 */
extern i32 panic (RO i8 *fmt,...);
extern u32 find_free_page (void);

void map_vaddr2paddr (u32 ttb_base,u32 vaddr,u32 paddr,u32 ap,i32 flush)
{
    if ( (paddr & 0xFFF) || (vaddr & 0xFFF))
        panic ("trying to map non-4K alignment vaddr 0x%0x \n",vaddr);

    VAR u32 *page_dir = (void*)0,*page_tbl = (void*)0;
    u32 page = 0,i;
    u32 l1_idx = vaddr>>20,l2_idx = (vaddr>>12)&0xFF;

    page_dir = (VAR u32 *)ttb_base;
    if (!(page_dir[l1_idx] & ~(0x3FF))) {                       /* page_dir empty */
        i = l1_idx & ~3;
        /* if one of 4M alignment L1 entry is empty
         * let 4-continuous entries ptr to the same 
         * page ,and then set those offset,any way
         * the 4entries must be zero,or else panic !
         */
        if ((page_dir[i+0] & ~0x3FF) || (page_dir[i+1] & ~0x3FF)
        ||  (page_dir[i+2] & ~0x3FF) || (page_dir[i+3] & ~0x3FF) )
        {
            panic ( "page dir corrupts with l1_idx %d ,l2_idx %d !\n"
                    ,l1_idx,l2_idx );
        }
        if (!(page = find_free_page ())) 
            panic ( "no more free page !\n" );                  /* alloc a page page dir*/
        wordset ((void*)page,AP_FAULT_ALL|CB|TTB1_SPG,0x1000);  /* we'd better clean it up ! */
        page = v2p(page);
        page_dir[i+0] = (page + 0x000)|DOMAIN_SYS|TTB0_COARSE;  /* small page 1st 1KB */
        page_dir[i+1] = (page + 0x400)|DOMAIN_SYS|TTB0_COARSE;  /* small page 2nd 1KB */
        page_dir[i+2] = (page + 0x800)|DOMAIN_SYS|TTB0_COARSE;  /* small page 3rd 1KB */
        page_dir[i+3] = (page + 0xC00)|DOMAIN_SYS|TTB0_COARSE;  /* small page 4th 1KB */

        page_tbl = (VAR u32*)(p2v((page_dir[l1_idx] & ~(0x3FF))));
        page_tbl [l2_idx] = paddr|ap|CB|TTB1_SPG; 
    } else {
        page_tbl = (VAR u32*)(p2v((page_dir[l1_idx] & ~(0x3FF))));
        page_tbl [l2_idx] = paddr|ap|CB|TTB1_SPG; 
    }
    /* safe is better than sorry 
     * clean the whole dcache
     */
    if (flush) {
        sync_dcaches ();                                        /* sync dcache -> external mem */
        invalidate_tlb (vaddr);    
    }
}
