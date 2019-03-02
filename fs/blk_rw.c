#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <ecasey/fs.h>


extern i32 rdread (void *buf,u32 blk_nr,u32 blks_cnt);
extern i32 rdwrite (void *buf,u32 blk_nr,u32 blks_cnt);
extern i32 sd_rw (u8 rw,u32 blk_nr,u32 blks_cnt,void *buf);
extern u32 nand_rw (u8 rw,u32 blkno,void *buf);

LOCAL i32 ram_rw (u8 rw,u32 blk_nr,u32 blks_cnt,void *buf)
{
    if (READ == rw) 
        rdread (buf,blk_nr,1);
    return (rdwrite (buf,blk_nr,1));
}

i32 ll_rw_blk (u8 rw,u16 dev_no,u32 blk_nr,u32 blks_cnt,void *buf)
{
    switch (dev_no)
    {
        case DEV_RAM:
            return (ram_rw (rw,blk_nr,blks_cnt,buf));
        case DEV_NAND:
            return (nand_rw (rw,blk_nr,buf));
        case DEV_SDCARD:
            return (sd_rw (rw,blk_nr,blks_cnt,buf));
        default:
            return (0);
    }
    return (0);
}
