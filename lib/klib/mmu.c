#include <2440/2440addr.h>
#include <2440/drvs.h>
#include <2440/ports.h>
#include <2440/cache.h>
#include <assert.h>
#include <string.h>
#include <ecasey/kernel.h>
#include <sys/types.h>

/* segment map set */ 
void map_seg_dir  (u32 paddr,u32 vaddr,u32 size,
                   u32 ttb_base,u32 page_dir_attr)
{
    if (size & 0xFFFFF)   
        return ;                 /* 1MB align chk*/
    VAR u32 *_page_dir = (VAR u32*)ttb_base;   
    _page_dir += (vaddr >> 20);
    u32  i = 0,nr = size >> 20;

    for (;i < nr ; i++,paddr += 0x100000)           /* 1MB step */
        _page_dir[i] = page_dir_attr| (paddr & 0xFFF00000);
}

/* map a 1K (256 entries) */
void map_page_tbl (u32 page_tbl,u32 page_tbl_attr,u32 paddr)
{
    i32 i = 0;
    VAR u32 *_page_tbl = (VAR u32*) page_tbl;

    for (; i < 256 ; i++,paddr += 0x1000)           /* 4KB step */ 
        _page_tbl[i] = page_tbl_attr | paddr;
}

/* 4k SMALL page map set */
void map_page_dir  (u32 paddr,u32 vaddr,u32 size,
                    u32 ttb_base,u32 page_dir_attr,
                    u32 page_tbl,u32 page_tbl_attr)
{
    if (size & 0xFFFFF)    return ;                 /* 1MB align chk*/

    VAR u32 *_page_dir = (VAR u32*)(ttb_base);   
    _page_dir += (vaddr>>20);
    u32  i = 0,nr = size >> 20;              /* 64 * 1M */
    VAR u32 _page_tbl = page_tbl;

    for (; i < nr ; i++,_page_tbl += 0x400,paddr += 0x100000)
    {
        _page_dir [i] = page_dir_attr | _page_tbl;  /* */
        map_page_tbl (_page_tbl,page_tbl_attr,paddr);
    }
}  

void *wordset (void *s,u32 w,i32 size)
{
    u32 *p = (u32*)s;
    while ((size -= 4) >= 0) *p ++ = w;
    return (s);
}

/* init mmu  */
void start_mmu (void)
{
    wordset ((void*)TTB_BASE,DOMAIN_FAULT|NCNB|TTB0_COARSE,TTB_SIZE);
    /*----------vaddr-paddr---size----ttb_base ---attr-----*/
    map_seg_dir (0x00,0x00,0x4000000,TTB_BASE,NORMAL_SEG_ATTR);

    /* SDRAM mapping */
    map_page_dir(RAM_START,RAM_START,4<<20 ,
                 TTB_BASE,
                 DOMAIN_SYS|TTB0_COARSE ,
                 PAGE_TBL_P,
                 AP_RW_ALL | CB | TTB1_SPG );
    /* FSR mapping */
	map_seg_dir (0x40000000,0x40000000,0x18000000,TTB_BASE,NORMAL_SEG_ATTR);

    /* mapping sdram 1M - 4M to kernel 3G+1M - 3G + 4M */
    map_page_dir(RAM_START,KERNEL_SPACE_START,4<<20 ,
                 TTB_BASE,
                 DOMAIN_SYS|TTB0_COARSE ,
                 PAGE_TBL_V,
                 AP_RW_ALL | CB | TTB1_SPG );
    /* well,to move kernel space ,I need to map port to high 3G address again 
     * port map address space takes 3 banks total 384M ,I put them at 
     * 0xE0000000 ,the last bank of 3G - 4G is used for IRQ  portion.....
     */
    map_seg_dir (0x48000000,PORTS_MAP_START,0x18000000,TTB_BASE,NORMAL_SEG_ATTR);

    /* vector page maped done ! */
    invalidate_idcache ();                              /* invalidate id cache */
    drain_wb ();                                        /* drain write buffer on armv4t */
    invalidate_tlbs();                                  /* flush tlb */
    set_ttb_base(TTB_BASE);                             /* use new ttb_base */   
    set_domain (0x55555550|(SYS_PL<<2)|(FAULT_PL<<0));  /* 3 domains fault 0,sys 1  usr 2 -15 */
    set_robin_replacement ();                           /* replacement robin*/
    set_little_endian ();                               /* cpu little endian operation */
    set_low_vector ();                                  /* vector 0x00000000 */
    align_fault_on ();                                  /* alignment fault chk on */
    mmu_on();                                           /* start mmu */
    icache_on();                                        /* start icache */
    dcache_on();                                        /* start dcache */
}

void sync_dcaches (void)
{
    i32 i = 0,j = 0;

    for (j = 0;j < 8;j ++){
        for (i = 0;i < 64; i++)                     
            clr_invalidate_dcache ( (i<<26)|(j<<5));
    }
}

