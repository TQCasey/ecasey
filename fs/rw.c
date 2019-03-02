#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <ecasey/fs.h>
#include <errno.h>

extern MEM_SUPER_BLOCK *get_sblk (u16);
extern int crash (const char *fmt,...);
extern BUFFER_HEAD *get_blk (u16,u32);
extern void brelse (BUFFER_HEAD*);
extern BUFFER_HEAD* breada (u16 dev_no,u32 blk_no ,...) ;
extern u32 bmap (MEM_INODE *m_inode,u32 file_blk_no) ;
extern u32 create_block (MEM_INODE *m_inode,u32 file_blk_no,u8 *created) ;
extern BUFFER_HEAD *bread (u16 dev_no,u32 blk_no) ;


extern u32 file_rd (MEM_INODE * inode,file_st* this_file,i8 * buf,i32 count);
extern u32 file_wr (MEM_INODE *m_wr,file_st *this_file,i8 *buf,i32 cnt) ;

i32 do_lseek (u32 pid,i32 fd,u32 offset,i32 origin)
{
	file_st * file;
	i32 tmp;

	if ((fd < 0) || (fd >= MAX_FILPS) 
    || !(file = pfilp(pid,fd)) || !(file->f_inode_ptr)) 
		return (-1);
	switch (origin) 
    {
		case 0:         /* from start */
			if (offset < 0) return (-1);
			file->f_pos = offset;       
			break;
		case 1:         /* from current pos*/
			if (file->f_pos+offset < 0) 
                return (-1);
			file->f_pos += offset;
			break;
		case 2          /* from tail */:
			if ((tmp=file->f_inode_ptr->i_size+offset) < 0)
				return (-1);
			file->f_pos = tmp;
			break;
		default:
			return (-1);
	}
	return (file->f_pos);
}

i32 do_read (u32 pid,i32 fd,i8 * buf,i32 count)
{
	file_st * file;
	MEM_INODE * inode;

	if ((count <= 0) || (fd < 0) || (fd >= MAX_FILPS) 
    || !(file = pfilp(pid,fd)) || !(inode = file->f_inode_ptr)) 
		return (0);
    /* mode cases */
    if (S_ISDIR(inode->i_mode)) 
        return (0);
	if (S_ISREG(inode->i_mode)) {
		if (count+file->f_pos > inode->i_size)
			count = inode->i_size - file->f_pos;
		if (count <= 0) 
            return (0);
		return (file_rd (inode,file,buf,count));
	}
	return (0);
}

i32 do_write (u32 pid,i32 fd,i8 * buf,i32 count)
{
	file_st * file;
	MEM_INODE * inode;

	if ((count <= 0) || (fd < 0) || (fd >= MAX_FILPS) 
    || !(file = pfilp(pid,fd)) || !(inode = file->f_inode_ptr)) 
		return (0);
    /* mode cases */
    if (S_ISDIR(inode->i_mode)) 
        return (0);
	if (S_ISREG(inode->i_mode))  
		return (file_wr(inode,file,buf,count));
	return (-EBADWR);
}
