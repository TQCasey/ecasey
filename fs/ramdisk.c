#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <ecasey/fs.h>
#include <2440/ports.h>

LOCAL   u8  *ramdsk;

#define NR_RAMDSK_INODES    (0x100)
#define NR_RAMDSK_ZONES     (192)
#define NR_RAMDSK_IMAP      (1)
#define NR_RAMDSK_BMAP      (1)

extern BUFFER_HEAD* bread (u16,u32);
extern void brelse (BUFFER_HEAD *);
extern int crash (const char *fmt,...) ;
extern void warn (RO i8 *fmt,...);

extern MEM_INODE *iget (u16,u32);
extern void iput (MEM_INODE *);
extern u32 new_block (u16);
extern void free_block (u16,u32);
extern void put_sblk (u16 dev_no) ;

i32 rdread (void *buf,u32 blk_nr,u32 blks_cnt)
{
    if (!buf || !blks_cnt 
    || (((blk_nr + blks_cnt)<<BLK_SIZE_SFT) >= RAMDSK_SIZE))
        return (0);
    /* lseek */
    memcpy (buf,ramdsk + (blk_nr << BLK_SIZE_SFT),blks_cnt << BLK_SIZE_SFT);
    return (blks_cnt << BLK_SIZE_SFT);
}

i32 rdwrite (void *buf,u32 blk_nr,u32 blks_cnt)
{
    if (!buf || !blks_cnt 
    || (((blk_nr + blks_cnt)<<BLK_SIZE_SFT) >= RAMDSK_SIZE))
        return (0);
    /* lseek */
    memcpy (ramdsk + (blk_nr << BLK_SIZE_SFT),buf,blks_cnt << BLK_SIZE_SFT);
    return (blks_cnt << BLK_SIZE_SFT);
}

i32 create_ramdsk (void)
{
    ramdsk = (u8*)(USR_RAMDSK_BUFFER);

    printf ( "$ > Source %s \n",(u8*)(ramdsk + RAMDSK_SIZE - 0x1000));
    return (0);
}
