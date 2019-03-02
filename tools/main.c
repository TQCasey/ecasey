#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <types.h>
#include <fs.h>
#include <stat.h>       /* this is conflict with system hdr,no problem */
#include <fcntl.h>

#define BYTES_PER_PAGE		(2048)      /* 2K/page */
#define SP_BYTES_PER_PAGE	(64)        /* 64 secondry bytes */
#define PAGES_PER_BLK		(64)        /* 64 pages/block */
#define BLKS_PER_NANDF		(2048)      /* 2K blocks/nand flash */

#define dsp printf

FILE *fp,*ini ;

LOCAL i32 blk_rw (u8 rw,u32 blk_nr,u32 blks_cnt,void *buf)
{
    if (!buf || !blks_cnt || fseek (fp,blk_nr << 12,SEEK_SET))
        return (0);
    if (rw == WRITE)
        return (fwrite (buf,1,blks_cnt<<12,fp));
    return (fread (buf,1,blks_cnt<<12,fp));
}

RO i8 *fmt[] = {
    "n_inodes=%d\n",
    "n_zones=%d\n",
    "n_imaps=%d\n",
    "n_bmaps=%d\n",
};

#define NR_OBJS     (sizeof(fmt)/sizeof(fmt[0]))

int main (int argc,char *argv[])
{
    if (argc != 3) {
        printf ( "usage : %s [input_file] [ini_file]\n",argv[0] );
        return (-1);
    }
    /* block file */
	if (!(fp = fopen (argv[1],"rb+"))) {
        fprintf ( stderr,"rw operation failed !\n" );
        return (0);
    }
    /* init file */
    if (!(ini = fopen (argv[2],"rb"))) {
        fprintf ( stderr,"read operation failed !\n" );
        return (0);
    }
    u32 i = 0,blk_nr = 2;
    u32 buf[BLK_SIZE>>2],tmp[BLK_SIZE>>2];
    MEM_SUPER_BLOCK *sb = (MEM_SUPER_BLOCK*)buf;

    for (i = 0 ; i < NR_OBJS ; i++) 
        fscanf (ini,fmt[i],&buf[i]);
    sb->sb_first_datazone = 2 + sb->sb_imap_blocks + sb->sb_zmap_blocks + 
        ((sb->sb_ninodes * sizeof(DEV_INODE) >> BLK_SIZE_SFT));
    sb->sb_log_zone_size = 0;
    sb->sb_max_size = sb->sb_zmap_blocks * (BLK_SIZE << 3) * BLK_SIZE;
    sb->sb_magic = CASEY_MAGIC;
    sb->sb_pos = 0;

    blk_rw (WRITE,1,1,(void*)buf);      /* write sblk */

    for (i = 0 ; i < sb->sb_imap_blocks ; i ++ ,blk_nr ++) {
        memset (tmp,0,BLK_SIZE);
        if (blk_nr == 2)
            tmp[0] = 0x01;
        blk_rw (WRITE,blk_nr,1,(void*)tmp);
    }
    for (i = 0 ; i < sb->sb_zmap_blocks ; i ++ ,blk_nr ++) {
        memset (tmp,0,BLK_SIZE);
        if (blk_nr == sb->sb_imap_blocks + 2)
            tmp [0] = 0x01;
        blk_rw (WRITE,blk_nr,1,(void*)tmp);
    }
    memset (tmp,0,sizeof(tmp));
    DEV_INODE *root = (DEV_INODE*)tmp;

    root->i_mode    = I_DIRECTORY;
    root->i_uid     = UID;
    root->i_size    = BLK_SIZE;
    root->i_mtime   = CUR_TIME;
    root->i_gid     = GID;
    root->i_nlinks  = 2;
    root->i_magic   = I_MAGIC;
    root->i_zone[0] = sb->sb_first_datazone;
    blk_rw (WRITE,2+sb->sb_imap_blocks+sb->sb_zmap_blocks,1,(void*)tmp);    /* dev_inode */

    memset (tmp,0,sizeof(tmp));
    DIR_ENTRY *bh_de_ptr = (DIR_ENTRY *)tmp;

    bh_de_ptr[0].inode_nr   = ROOT_IND_NR;
    bh_de_ptr[0].name[0]    = '.';
    bh_de_ptr[0].name[1]    = '\0';

    bh_de_ptr[1].inode_nr   = ROOT_IND_NR;
    bh_de_ptr[1].name[0]    = '.';
    bh_de_ptr[1].name[1]    = '.';
    bh_de_ptr[1].name[2]    = '\0';

    bh_de_ptr[2].inode_nr   = ROOT_IND_NR;
    strcpy (bh_de_ptr[2].name,"LOVE");

    blk_rw (WRITE,sb->sb_first_datazone,1,(void*)tmp);

    fclose (fp);
    fclose (ini);
    return (0);
}
