#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <ecasey/fs.h>
#include <errno.h>

extern MEM_INODE *namei (RO i8 *path) ;
extern void iput (MEM_INODE *m_puti) ;

LOCAL void cp_stat (MEM_INODE *m_inode,struct stat *p)
{
    p->st_dev   = m_inode->i_dev;
    p->st_ino   = m_inode->i_num;
    p->st_gid   = m_inode->i_gid;
    p->st_uid   = m_inode->i_uid;
    p->st_size  = m_inode->i_size;
    p->st_mode  = m_inode->i_mode;
    p->st_nlink = m_inode->i_nlinks;
    p->st_rdev  = (u16)m_inode->i_zone [0];
    p->st_atime = m_inode->i_acc_time;
    p->st_ctime = m_inode->i_crt_time;
    p->st_mtime = m_inode->i_mtime;
}

i32 do_stat (RO i8 *fname,struct stat *p)
{
    MEM_INODE *m_inode ;

    if (!(m_inode = namei (fname))) 
        return (-ENOPATH);
    cp_stat (m_inode,p);
    iput (m_inode);
    return (0);
}

i32 do_fstat (i32 pid,i32 fd,struct stat *p) 
{
    file_st *file ;
    MEM_INODE *m_inode;

    if (fd >= MAX_FILPS || !(file = pfilp (pid,fd)) || !(m_inode = file->f_inode_ptr)) 
        return (-EBADFD);
    cp_stat (m_inode,p);
    return (0);
}
