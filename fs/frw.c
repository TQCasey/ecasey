#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <ecasey/fs.h>

extern MEM_SUPER_BLOCK *get_sblk (u16);
extern int crash (const char *fmt,...);
extern BUFFER_HEAD *get_blk (u16,u32);
extern void brelse (BUFFER_HEAD*);
extern BUFFER_HEAD* breada (u16 dev_no,u32 blk_no ,...) ;
extern u32 bmap (MEM_INODE *m_inode,u32 file_blk_no) ;
extern u32 create_block (MEM_INODE *m_inode,u32 file_blk_no,u8 *created) ;
extern BUFFER_HEAD *bread (u16 dev_no,u32 blk_no) ;

u32 file_rd (MEM_INODE * inode,file_st* this_file,i8 * buf,i32 count)
{
	i32 left,chars,nr;
	BUFFER_HEAD * bh;

    if (!this_file || ((left = count) <= 0) || !buf) 
        return (0);
	while (left > 0) {
		if ((nr = bmap (inode,(this_file->f_pos)>>BLK_SIZE_SFT))) { /* get block nr */
			if (!(bh = bread (inode->i_dev,nr)))
				break;
        } else bh = NULL;

		nr = this_file->f_pos & BLK_SIZE_MSK;
        chars = BLK_SIZE - nr;
        if (chars > left)
            chars = left;
		this_file->f_pos += chars;
		left -= chars;
		if (bh) {
			i8 * p = (i8*)(nr + bh->bh_buf);
			while (chars-->0)
                *buf ++ = *p ++;
			brelse(bh);
		} else {
            /* bread may failed ,however blk_no is okay ! */
			while (chars-->0)
                *buf ++ = 0x00;
		}
	}
	inode->i_acc_time = CUR_TIME;
	return ((count - left)?(count - left) : 0);
}

u32 file_wr (MEM_INODE *m_wr,file_st *this_file,i8 *buf,i32 cnt) 
{
    if (!this_file || (cnt <= 0) || !buf) 
        return (0);
    u64 pos = 0;

    if (this_file->f_flags & O_APPEND) /* append */
        pos = m_wr->i_size;     /* move file ptr to the tail */
    else
        pos = this_file->f_pos; /* current pos */

    BUFFER_HEAD *bh;
    u32 blk_no = 0,offset = 0;
    i32 chars = 0,written = 0;
    i8 *p = NULL;
    u8 created = false;

    while (cnt > 0) {
        if (!(blk_no = create_block (m_wr,(this_file->f_pos>>BLK_SIZE_SFT),&created))) 
            break;
        if (!(bh = bread (m_wr->i_dev,blk_no)) ) 
            break;
        offset = pos & BLK_SIZE_MSK;
        chars = BLK_SIZE- offset;
        if (chars > cnt) 
            chars = cnt ;
        p = (i8*)(bh->bh_buf + offset) ;
        /* any way ,append file here will increase file size,which the file size 
         * will be changed ,if not append but wrtie more size (> isize) bytes into 
         * this file ,we need to renew file size just once (later) ...
         */
        pos += chars ;
        if ( pos > m_wr->i_size ) {
            m_wr->i_size = pos;
            m_wr->i_dirt = true;
        }
        cnt -= chars ;
        written += chars ;
        this_file->f_pos += chars;
        while ( chars -- > 0 ) 
            *p ++ = *buf ++;
        bh->bh_dirt = true;
        brelse (bh);
    }
    m_wr->i_mtime = CUR_TIME;
    /* if it is append,size already changed 
     * so no need to change again 
     */
    if (!(this_file->f_flags & O_APPEND )) { 
        this_file->f_pos = pos;
        m_wr->i_crt_time = CUR_TIME;
    }
    return (written);
}
