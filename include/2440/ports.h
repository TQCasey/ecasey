/* FOR 2440 ASM HDR *(LOW LEVEL INIT)
 * BY CASEY this can be used before swithing 
 * 2013 - 3 - 24 
 */
#ifndef __PORTS_H__
#define __PORTS_H__

/* the reason we put here is to share with asm file 
 * better to put nothing but macros 
 */
#define v2p(x)              ((x)-0x90000000)
#define p2v(x)              ((x)+0x90000000)

/* img offset in nand flash
 */
#define LOADER_IMG          (0x1000)            /* the 1st blk */
#define LOADER_SIZE         (0x8000)            /* 32K */
#define LOADK_START         (0x30010000)        /* load kernel entry */ 

#define VECT_IMG            (0x4000)            /* 16K */
#define VECT_SIZE           (0x1000)            /* 4K */

#define KERNEL_IMG          (0x20000)           /* the 2nd blk */
#define KERNEL_IMG_SIZE     (0x10000)           /* 64K */

#define FS_IMG              (0x40000)           /* the 3rd blk */
#define FS_IMG_SIZE         (0x10000)           /* 64K */

#define SH_IMG              (0x60000)           /* the 4th blk */
#define SH_IMG_SIZE         (0x8000)            /* 32K */

#define MISC_INFO           (0x80000)           /* the 5th blk is for misc info */
#define MISC_INFO_SIZE      (0x20000)           /* a block size */

#define RAM_START           (0x30000000)
#define RAM_SIZE            (0x4000000)
#define KERNEL_SPACE_START  (0xC0000000)        /* 3G */
#define KERNEL_SPACE_SIZE   (1<<30)             /* 1G */

#define TTB_BASE            (RAM_START)         /* tmp page dir start */     
#define TTB_SIZE            (0x4000)            /* 16K */   

#define PAGE_TBL_P          (TTB_BASE+TTB_SIZE)                 
#define PAGE_TBL_P_SIZE     (0x1000)            /* 4M : 4K */

#define PAGE_TBL_V          (PAGE_TBL_P+PAGE_TBL_P_SIZE)
#define PAGE_TBL_V_SIZE     (0x1000)            /* 4M : 4K */

#define KERNEL_CODE_START   (0xC0100000)        /* 3G start */
#define KERNEL_STACK        (0xC0100000)        /* stack for kernel */ 
#define KERNEL_STACK_SIZE   (0x8000)            /* 32K */

/* new kernel page dir (bank 6: sdram start at 64k) 
 * 0 - 0x10000 in sdram is for old page dir & tbls 
 */
#define KERNEL_TTB          (KERNEL_SPACE_START+(0x10000))
#define KERNEL_PAGE_TBL     (KERNEL_TTB+TTB_SIZE)
/* prepare a page for vect */
#define VECT_PAGE_TBL       (KERNEL_PAGE_TBL+0x10000)   
#define VECT_PAGE           (VECT_PAGE_TBL+0x1000)

/* usr address space */
#define USR_ENTRY           (0x60000000)
#define USR_SP              (0x60100000)
#define USR_SP_SIZE         (0x2000)            /* 8K */

/* mail post/pick buf */
#define SEND_BUF            (0x60000000 - 0x2000)
#define RECV_BUF            (0x60000000 - 0x1000)

#define HEAP_AREA(x)        (((x) >= 0x60100000) && ((x) < 0xC0000000))
#define STACK_AREA(x)       (((x) >= 0x60000000) && ((x) < 0x60100000))
#define VMA_AREA(x)         (HEAP_AREA(x) || STACK_AREA(x))

#define PORTS_MAP_START     (0xE0000000)        /* PORT AREA at 3G - 4G bank 4 */

/* vector page map */
#define VECTOR_TBL          (0xFFFF0000)        
#define VECTOR_TBL_SIZE     (0x400)             /* 1K */

#define IRQ_TABLE           (VECTOR_TBL+VECTOR_TBL_SIZE)
#define IRQ_TABLE_SIZE      (0x400)             /* 1K irq table size */
#define _ISR_STARTADDRESS	(IRQ_TABLE)         /* ISR tbl */ 

/* kmalloc kernel mem map */
#define MAP_AREA_START      (0xC0400000)        /* 3G + 4M start */      
#define MAP_AREA_END        (0xC4000000)        /* 3G + 64M end */ 
#define MAP_AREA_SIZE       (MAP_AREA_END - MAP_AREA_START)
#define LOW_MEM_SIZE        (4<<20)             /* 4M for kernel */

/* TODO :FIXME when struct page changed !! */
#define PAGE_STRUCT_SIZE    (28)  
#define PAGE_STRUCTS_NR     (MAP_AREA_SIZE>>12) /* 4K */

/* struct page mem area */
#define PAGE_STRUCTS_START  (VECT_PAGE+0x1000)
#define PAGE_STRUCTS_SIZE   (PAGE_STRUCTS_NR*PAGE_STRUCT_SIZE)
#define PAGE_STRUCTS_END    (PAGE_STRUCTS_START+PAGE_STRUCTS_SIZE)

/* Video buffer */
#define LCD_BUFFER          (PAGE_STRUCTS_END+0x100)
#define LCD_BUF_SIZE        (0x3FC00)           /* 255K lcd_buf */

#define TL_BUFFER           (0xC0200000)        /* translation layer buffer */
#define TL_BUF_SIZE         (0x100000)          /* 1M  */
#define BLK_BUF_SIZE        (0x20000)           /* 128 K */
#define BLK_BUF_SIZE_SFT    (17)

#define FS_BUFFER           (0xC0300000)
#define FS_BUFFER_SIZE      (0x100000)
#define USR_FS_BUFFER       (0x50000000)

#define RAMDSK_BUFFER       (0xC0140000)
#define RAMDSK_SIZE         (0xC0000)
#define USR_RAMDSK_BUFFER   (0x50100000)
#define RAMDISK_IMG_BLK_NO  (40)

/* cache head start */
#define CACHE_ST_SIZE       (sizeof(tl_cache_st))
#define CACHE_ST_NR         (TL_BUF_SIZE/BLK_BUF_SIZE)
#define TL_CACHE_HEAD       (LCD_BUFFER+LCD_BUF_SIZE)
#define TL_CACHE_END        (TL_CACHE_HEAD+(CACHE_ST_SIZE)*(CACHE_ST_NR))

/* logical blk map list */
#define MAP_TAB             (TL_CACHE_END)
#define MAP_TAB_SIZE        (sizeof(map_tab_st))

/* mode const */
#define USR_MODE            (0x10)
#define FIQ_MODE            (0x11)
#define IRQ_MODE            (0x12)
#define SVC_MODE            (0x13)
#define ABT_MODE            (0x14)
#define UND_MODE            (0x15)
#define SYS_MODE            (0x1F)
#define MODE_MASK           (0x1F)
#define NO_INTR             (0xC0)

#define NO_IRQ              (1<<7)
#define NO_FIQ              (1<<6)  

#endif
