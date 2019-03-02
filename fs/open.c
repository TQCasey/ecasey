#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <ecasey/fs.h>
#include <errno.h>

file_st file_table[MAX_OPEN_FILES];
file_st *filp[NR_TASKS][MAX_FILPS];     /* file operator pointer */ 

extern i32 open_namei (RO i8*,u16,u16,MEM_INODE**);
extern i32 crash (RO i8*,...);
extern void iput (MEM_INODE *);

void init_ftbl (void)
{
    memset (file_table,0,sizeof(file_table));
    memset (filp,0,sizeof(filp));
}

file_st *get_free_ftbl (void)
{
    i32 i = 0;

    for (i = 0; i < MAX_OPEN_FILES ; i ++) 
        if (!file_table[i].f_cnt) 
            return (file_table+i);
    return (NULL);
}

LOCAL i32 get_free_fd (i32 pid)
{
    i32 i = 0;

    for (i = 0; i < MAX_FILPS ; i++) 
        if (!filp[pid][i])
            return (i);
    return (-1);
}

i32 do_open (i32 pid,RO i8 *path,i32 flag,i32 mode)
{
    i32 fd = get_free_fd (pid);
    file_st *ftbl = get_free_ftbl ();

    if ((fd < 0) || !ftbl) 
        return (-ENOSRC);
    MEM_INODE *m_inode;
    if (open_namei (path,flag,mode,&m_inode) < 0){
        pfilp (pid,fd) = NULL;
        ftbl->f_cnt = 0;
        return (-EOPNFAILD);
    }
    ftbl->f_mode = m_inode->i_mode;
    ftbl->f_flags = flag;
    ftbl->f_inode_ptr = m_inode;
    ftbl->f_pos = 0;
    (pfilp (pid,fd) = ftbl)->f_cnt  ++;
    return (fd);
}

/* close file discriptor */
i32 do_close (i32 pid,i32 fd)
{
    if (fd >= MAX_FILPS || fd < 0) 
        return (-1);
    file_st *this_file = pfilp (pid,fd);
    if (this_file == NULL) 
        return (-1);
    pfilp (pid,fd) = NULL;
    if (!this_file->f_cnt) 
        return (-EOPNFAILD);
    if (--this_file->f_cnt) 
        return (0);
    iput (this_file->f_inode_ptr);
    if (!this_file->f_cnt)
        memset (this_file,0,sizeof(file_st));
    return (0);
}
