#ifndef __FS_H__
#define __FS_H__

/* fs umsg */
#define FS_OPEN         0x0100
#define FS_CLOSE        0x0101
#define FS_SEEK         0x0102
#define FS_READ         0x0103
#define FS_WRITE        0x0104

#include <sys/types.h>

typedef struct tagBUFFER_HEAD{
    u8*         bh_buf ;                
    u32         bh_blk_nr;                  
    u16         bh_dev;                     
    u8          bh_valid;                   
    u8          bh_dirt;                    /* wb */
    u8          bh_cnt;                     
    u8          bh_locked;                  /* writer lock it down */
	u8			bh_hashed;
    struct      tagBUFFER_HEAD  *hash_prev; /* hash prev */
    struct      tagBUFFER_HEAD  *hash_next; /* hash next */ 
    struct      tagBUFFER_HEAD  *free_prev; /* prev free */
    struct      tagBUFFER_HEAD  *free_next; /* next free */
}BUFFER_HEAD,*LPBUFFER_HEAD;

/* dev inode struct def here*/  
typedef struct tagDEV_INODE{
    u16         i_mode;                     
    u16         i_uid;                  
    u32         i_size;                     /* file size (inbytes) */
    u32         i_mtime;                    /* latest modified time */
    u16         i_gid;                      /* group id */
    u16         i_nlinks;                   /* links for files */
    u32         i_magic;
    u32         i_zone[9];                  /* zone block dir */
    u16         unused[4];                  /* reserved for ecasey */
}DEV_INODE,*LPDEV_INODE;

/* memory inode struct def here */
typedef struct tagMEM_INODE{
    u16         i_mode;                     
    u16         i_uid;                  
    u32         i_size;                     /* file size (inbytes) */
    u32         i_mtime;                    /* latest modified time */
    u16         i_gid;                      /* group id */
    u16         i_nlinks;                   /* links for files */
    u32         i_magic;
    u32         i_zone[9];                  /* zone block dir */
    u16         unused[4];                  /* reserved for ecasey */
    /* these are ONLY in memory */
    u32         i_acc_time;                 /* latest access time */
    u32         i_crt_time;                 /* create time */
    u32         i_num;                      /* inr */
    u16         i_dev;                      /* dev */
    u16         i_cnt;                      /* cnt */
    u8          i_locked;                   /* writer locked */
    u8          i_dirt;                     /* dirt */
    u8          i_pipe;                     /* pipe type */
    u8          i_mounted;                  /* mounted flag */
    u8          i_seek;                     /* seek flag */
    u8          i_valid;                    /* valid */
}MEM_INODE,*LPMEM_INODE;

/* dev super block defs here */
typedef struct tagDEV_SUPER_BLOCK{
    u32         sb_ninodes;                  // total inodes 
    u32         sb_nzones;                   // totol logic blks 
    u32         sb_imap_blocks;              // inode map blks 
    u32         sb_zmap_blocks;              // logical map blks 
    u32         sb_first_datazone;           // first data logic blk nr
    u32         sb_log_zone_size;            // log2(data_blks/logic blks)
    u32         sb_max_size;                 // file max size
    u32         sb_magic;                    // fs magic num
    u32         sb_pos;                      
}DEV_SUPER_BLOCK,*LPDEV_SUPER_BLOCK;


#define NR_IMAP_BLKS    8
#define NR_ZMAP_BLKS    8
#define NR_SBLKS        8

/* mem super block defs here */
typedef struct tagMEM_SUPER_BLOCK{
    u32         sb_ninodes;                  /* total inodes */
    u32         sb_nzones;                   /* totol logic blks */
    u32         sb_imap_blocks;             
    u32         sb_zmap_blocks;              
    u32         sb_first_datazone;           /* first data blknr */
    u32         sb_log_zone_size;            /* log2(data_blks/logic blks)*/
    u32         sb_max_size;                 /* file max size */
    u32         sb_magic;                    /* fs magic num */
    u32         sb_pos;
    /* these are ONLY in memory*/
    BUFFER_HEAD *sb_imap_bh[NR_IMAP_BLKS];  // max imap blocks
    BUFFER_HEAD *sb_zmap_bh[NR_ZMAP_BLKS];    // max zmap blocks 
    u16         sb_dev;                      // dev num
    MEM_INODE   *sb_i_nr;                    // mounted fs root dir inode nr
    MEM_INODE   *sb_i_mounted;               // mounted fs inode nr
    u32         sb_time;                     // modified time
    u8          sb_ro;						 // read only flag
    u8          sb_dirt;                     // modified flag
    u8          sb_locked;                   // locked ?
}MEM_SUPER_BLOCK;

#define NAME_LEN        60
/* struct dir entry */
typedef struct tagDIR_ENTRY{
    u32         inode_nr;                   
    i8          name[NAME_LEN] ;            
}DIR_ENTRY,*LPDIR_ENTRY;

/* file_struct defs here */
#define MAX_OPEN_FILES  64
#define MAX_FILPS       32
typedef struct _file_st {
    u16         f_mode;                     // file opt mode
    u16         f_flags;                    // file opt flag ( open / rw )
    u16         f_cnt;                      // file descriptors count
    MEM_INODE   *f_inode_ptr;               // ptr to inode nr 
    u64         f_pos;                      // file pos 
} file_st;

#define BLK_SIZE        (4096)
#define INODES_PER_BLK  (BLK_SIZE/sizeof(DEV_INODE))
#define ENTRIES_PER_BLK (BLK_SIZE/sizeof(DIR_ENTRY))

#define ROOT_IND_NR     (0)
#define PWD_LEN         (64)

typedef struct _cw_st {
    u16         dev;
    u32         inr;
}cw_st;

struct proc_st {
    cw_st       root,cwd;
    i8          pwd [PWD_LEN];
    i32         pid;
};

#define CASEY_MAGIC     (0x05201314)
#define I_MAGIC         (0xCCAA)
#define NR_ZONES        (0xFA00)
#define NR_INODES       (0x40000)

#define BLK_BITS_SFT    (15)
#define BLK_BITS_MSK    (0x7FFF)

#define BLK_SIZE_SFT    (12)
#define BLK_SIZE_MSK    (0xFFF)

#define NR_BLK_OBJS		(BLK_SIZE>>2)
#define BLK_OBJ_SFT	    (10)
#define BLK_OBJ_MSK	    (0x3FF)

#define WRITE           0
#define READ            1

#define IND_EMPTY(x)    ((x) == 0)

#define DEV_NAND        (0x0100)
#define DEV_RAM         (0x0101)
#define DEV_SDCARD      (0x0102)
#define ROOT_DEV        (DEV_NAND)

#define CUR_TIME        (0x12345678)
#define UID             (0xCC)
#define GID             (0xAA)

#define NR_INODE_BUF    (128)                         

#define NR_TASKS        (256)
extern file_st *filp[NR_TASKS][MAX_FILPS];     /* file operator pointer */ 
#define pfilp(pid,fd)   (filp[pid][fd])


#endif
