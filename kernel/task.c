#include <2440/2440addr.h>
#include <ecasey/pcb.h>
#include <2440/ports.h>
#include <2440/drvs.h>
#include <2440/cache.h>
#include <ecasey/kernel.h>
#include <sys/types.h>


u32 NR_TASKS   = 0;
TASK*    task_st[MAX_TASKS] = {0};

void init_task_st (void)
{
    i32 i = 0;
    for (; i < MAX_TASKS; i ++)
        task_st[i] = 0;
}
LOCAL 
i32 find_free_pid (void)
{
    i32 i = 0; 
    for (; i < MAX_TASKS; i ++){
        if (!task_st[i])
            return (i);
    }
    return (-1);
}

extern void sync_dcaches (void);

void change_ttb_base (u32 ttb_base)
{
    asm VAR ("" ::: "memory"); /* barrier */
    sync_dcaches ();
    drain_wb ();
    invalidate_icache ();
    invalidate_dcaches ();
    set_ttb_base(ttb_base);                         
    asm  VAR ("nop" );
    asm  VAR ("nop" );         /* pipeline clean up */
    invalidate_tlbs();      
}

#define TTB_IS_ERR(x)       (((x)&0x03) == 0x00) 
#define TTB_IS_CPG_TBL(x)   (((x)&0x03) == 0x01) 
#define TTB_IS_SEG_TBL(x)   (((x)&0x03) == 0x02) 
#define TTB_IS_FPG_TBL(x)   (((x)&0x03) == 0x03)
#define TTB_EMPTY(x)        (((x)&~0x3FF) == 0)

extern i32 panic (i8 *fmt,...);
extern u32 find_free_page (void);
extern void *wordset (void *s,u32 w,i32 size);

void share_kernel_space (u32 dest_ttb,u32 vaddr,u32 size)
{
    if ((size & 0xFFFFF) || (vaddr & 0xFFFFF))                              /* 1M alignment */
        panic ( "trying to copy page tables with non-1M alignment !\n" );
    VAR u32 *_from_page_dir,*_to_page_dir;

    _from_page_dir = (VAR u32 *)(KERNEL_TTB);
    _to_page_dir = (VAR u32 *)dest_ttb;
    u32 l1_idx,i = 0,j = 0,k = 0,page;
    VAR u32 *_to_page_tbl,*_from_page_tbl;

    for (k = 0 ,size >>= 20 ; k < size ; k ++,vaddr += 0x100000 ) {
        l1_idx = vaddr>>20;
        if ( TTB_EMPTY (_from_page_dir[l1_idx]) )                           /* skip empty dir */ 
            continue;
        if ( TTB_IS_SEG_TBL (_from_page_dir [l1_idx]) )                     /* if it is section tbl */
            _to_page_dir[l1_idx] = _from_page_dir [l1_idx];                 /* copy too */
        else if ( TTB_IS_CPG_TBL (_from_page_dir[l1_idx]) ) {               /* if it is coarse page tbl */ 
            if ( !(_to_page_dir[l1_idx] & ~0x3FF) ) {                       /* if dest page dir is empty */
                i = l1_idx & ~3;
                if ((_to_page_dir[i+0] & ~0x3FF) || (_to_page_dir[i+1] & ~0x3FF)
                ||  (_to_page_dir[i+2] & ~0x3FF) || (_to_page_dir[i+3] & ~0x3FF) )
                {
                    panic ( "page dir corrupts with l1_idx %d !\n",l1_idx );
                }
                if (!(page = find_free_page ())) 
                    panic ( "no more free page !\n" );                      /* alloc a page page dir*/
                wordset ((void*)page,AP_FAULT_ALL|CB|TTB1_SPG,0x1000);      /* set all be fault  */
                page = v2p(page);
                _to_page_dir[i+0] = (page+0x000)|DOMAIN_SYS|TTB0_COARSE;    /* the 1st 1KB */
                _to_page_dir[i+1] = (page+0x400)|DOMAIN_SYS|TTB0_COARSE;    
                _to_page_dir[i+2] = (page+0x800)|DOMAIN_SYS|TTB0_COARSE;    
                _to_page_dir[i+3] = (page+0xC00)|DOMAIN_SYS|TTB0_COARSE;    /* the 4th 1KB */
            }
            _from_page_tbl = (VAR u32*)(p2v((_from_page_dir[l1_idx] & ~0x3FF)));
            _to_page_tbl = (VAR u32*)(p2v((_to_page_dir[l1_idx] & ~0x3FF)));
            /* continue copying .... */
            for (j = 0 ; j < 256 ; j++ )                                    /* 256 * 4K */
                if (_from_page_tbl[j] & ~0xFFF) 
                    _to_page_tbl[j] = _from_page_tbl[j];
        }
    }
}
extern void map_vaddr2paddr (u32 ttb_base,u32 vaddr,u32 paddr,u32 ap,i32 flush);
void init_task (u32 ttb,u32 img_entry)
{
    i32 pid = -1;
    if ( (pid = find_free_pid ()) < 0 ) {
        printk ( "no more pcb slot avilable ! \n" );
        return ;
    }
    TASK *pcb = (TASK*)find_free_page ();
    if (!pcb) {
        printk ( "no more free page avilable ! \n" );
        return ;
    }
    pcb->regs.sp    = USR_SP + USR_SP_SIZE; 
    pcb->regs.spsr  = SYS_MODE|NO_FIQ;
    pcb->regs.pc    = img_entry;

    pcb->state      = TS_RUN;
    pcb->pid        = pid;
    pcb->tick       = pcb->prio     = 1;
    pcb->ttb_base   = ttb;          /* ttb base ,16Kb alignment */
    pcb->head       = pcb->tail     =  pcb->next = (void*)0;
    pcb->mail.to    = NO_TASK;

    /* prepare for stack mem */
    u32 page = find_free_page ();
    map_vaddr2paddr (ttb,USR_SP,v2p(page),AP_RW_ALL,1);
    task_st [pid] = pcb;
    NR_TASKS ++ ;       
}

extern u32 find_free_page_dir ( void );
extern i32 strcmp ( RO i8 *str1,RO i8 *str2 ) ;
extern i32 get_order (u32 size);
extern void *get_free_pages(u32 flag,i32 order);

i32 createp (RO i8 *name,i32 prio,i32 state)
{
    i32 pid = -1;
    if ( (pid = find_free_pid ()) < 0 ) {
        printk ( "no more pcb slot avilable ! \n" );
        return (-1);
    }
    TASK *pcb = (TASK*)find_free_page ();

    pcb->regs.sp    = USR_SP + USR_SP_SIZE; 
    pcb->regs.spsr  = USR_MODE|NO_FIQ;
    pcb->regs.pc    = USR_ENTRY;

    u32 page_dir ;
    page_dir        = find_free_page_dir ();
    wordset ((void*)page_dir,DOMAIN_FAULT|NCNB|TTB0_COARSE,TTB_SIZE);
    pcb->ttb_base   = page_dir;
    pcb->state      = state & TS_MASK;
    pcb->pid        = pid;
    pcb->prio       = pcb->tick     = prio % PL_MAX;
    pcb->head       = pcb->tail     = pcb->next = (void*)0;
    pcb->mail.to    = NO_TASK;

    /* copy kernel 3G */
    share_kernel_space (page_dir,KERNEL_SPACE_START,KERNEL_SPACE_SIZE);

    i32 i = 0,order = -1;
    u32 page,vaddr,paddr;

    if (0 == strcmp (name,"fs") ) {
        pcb->regs.spsr = SYS_MODE | NO_FIQ;
        if ((order = get_order (FS_IMG_SIZE)) < 0)
            panic ( "no more free pages !\n" );
        if (!(page = (u32)get_free_pages (0,order)))             /* get buddy order & alloc page blk */
            panic ( "no more free pages !\n" );
        vaddr = USR_ENTRY,paddr = v2p(page);
        copy_nand_to_sdram (page,FS_IMG,FS_IMG_SIZE);
        for (i = 0 ; i < (FS_IMG_SIZE>>12) ; i++,vaddr += 0x1000,paddr += 0x1000) 
            map_vaddr2paddr (page_dir,vaddr,paddr,AP_RW_ALL,0);
        /* prepare 1M file system buffer */
        paddr = v2p(FS_BUFFER),vaddr = USR_FS_BUFFER;
        for (i = 0 ; i < (FS_BUFFER_SIZE>>12) ; i++,vaddr += 0x1000,paddr += 0x1000) 
            map_vaddr2paddr (page_dir,vaddr,paddr,AP_RW_ALL,0);
        /* prepare 768K ramdisk */
        paddr = v2p(RAMDSK_BUFFER),vaddr = USR_RAMDSK_BUFFER;
        for (i = 0 ; i < (RAMDSK_SIZE>>12) ; i++,vaddr += 0x1000,paddr += 0x1000) 
            map_vaddr2paddr (page_dir,vaddr,paddr,AP_RW_ALL,0);
    } else if (0 == strcmp (name,"sh") ) {
        pcb->regs.spsr = SYS_MODE | NO_FIQ;
        if ( (order = get_order (SH_IMG_SIZE)) < 0) 
            panic ( "no more free pages !\n" );
        if (!(page = (u32)get_free_pages (0,order)))             /* get buddy order & alloc page blk */
            panic ( "no more free pages !\n" );
        copy_nand_to_sdram (page,SH_IMG,SH_IMG_SIZE);
        vaddr = USR_ENTRY,paddr = v2p(page);
        for (i = 0 ; i < (SH_IMG_SIZE>>12) ; i++,vaddr += 0x1000,paddr += 0x1000) 
            map_vaddr2paddr (page_dir,vaddr,paddr,AP_RW_ALL,0);
    }
    /* stack */
    if ((order = get_order (USR_SP_SIZE)) < 0)
        panic ( "buddy order wrong  !\n" );
    if (!(page = (u32)get_free_pages (0,order)))                 /* get buddy order & alloc page blk */
        panic ( "no more free pages !\n" );

    vaddr = USR_SP,paddr = v2p(page);
    for (i = 0 ; i < (USR_SP_SIZE>>12) ; i++,vaddr += 0x1000,paddr += 0x1000) 
        map_vaddr2paddr (page_dir,vaddr,paddr,AP_RW_ALL,0); /* no flush */
    /* send buf */
    page = find_free_page ();
    map_vaddr2paddr (page_dir,SEND_BUF,v2p(page),AP_USR_RO_ALL,0);

    /* recv buf */
    page = find_free_page ();
    map_vaddr2paddr (page_dir,RECV_BUF,v2p(page),AP_USR_RO_ALL,0);

    task_st [pid] = pcb;
    NR_TASKS ++ ;       
    return (pid);
}

extern INLINE void * memset(void * s,i8 c,i32 count);
extern void map_page_dir (u32 paddr,u32 vaddr,u32 size,
                        u32 ttb_base,u32 page_dir_attr,
                        u32 page_tbl,u32 page_tbl_attr);
extern void map_seg_dir  (u32 paddr,u32 vaddr,u32 size,
                        u32 ttb_base,u32 page_dir_attr);

void rebuild_mmu (void)
{
    memset ((void*)KERNEL_TTB,0,TTB_SIZE);           
    map_page_dir(RAM_START,
                 KERNEL_SPACE_START,
                 RAM_SIZE,
                 v2p(KERNEL_TTB),
                 DOMAIN_SYS|TTB0_COARSE ,
                 v2p(KERNEL_PAGE_TBL),
                 AP_USR_RO_ALL | CB | TTB1_SPG );

    map_seg_dir (0x48000000,PORTS_MAP_START,0x18000000,
                 v2p(KERNEL_TTB),NORMAL_SEG_ATTR);
    /* ready for vector page  
     */
    u32 vect_page_tbl  = v2p(VECT_PAGE_TBL);
    u32 vect_page      = v2p(VECT_PAGE);  
    u32 vaddr          = 0xFFFF0000;

    VAR u32 *page_dir = (VAR u32*)(v2p(KERNEL_TTB));
    page_dir [ vaddr >> 20 ] = vect_page_tbl | DOMAIN_SYS | TTB0_COARSE ;
    VAR u32 *page_tbl = (VAR u32*)vect_page_tbl;
    page_tbl [ (vaddr >> 12) & 0xFF ] = vect_page | AP_RW_ALL |CB|TTB1_SPG; 

    change_ttb_base (v2p(KERNEL_TTB));
}
