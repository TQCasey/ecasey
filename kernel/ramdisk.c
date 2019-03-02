#include <2440/drvs.h>
#include <ecasey/kernel.h>
#include <assert.h>
#include <ecasey/pcb.h>
#include <2440/cache.h>
#include <sys/types.h>
#include <string.h>


extern i8 read_blk (void *buf,i32 blk_nr);

void read_ramdisk (void)
{
    i32 i = 0,blkno = RAMDISK_IMG_BLK_NO;
    void *to = (void*)RAMDSK_BUFFER;

    for (i = 0 ; i < (RAMDSK_SIZE>>BLK_BUF_SIZE_SFT) ; i++,blkno ++) {
        read_blk (to,blkno);
        to += BLK_BUF_SIZE;
    }
    strcpy ((void*)(RAMDSK_BUFFER+RAMDSK_SIZE-0x1000),"hello world");   /* just for a test */
}

