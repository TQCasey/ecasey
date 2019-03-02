#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <ecasey/fs.h>
#include <errno.h>

extern u32 bmap (MEM_INODE *m_inode,u32 file_blk_no) ;
extern u32 create_block (MEM_INODE *m_inode,u32 file_blk_no,u8 *created);
extern BUFFER_HEAD* bread (u16,u32);
extern void brelse (BUFFER_HEAD *);
extern void iput (MEM_INODE *);

extern BUFFER_HEAD *get_blk (u16 dev_no,u32 blk_no) ;
extern MEM_SUPER_BLOCK* get_sblk (u16 dev_no) ;

extern int crash (const char *fmt,...) ;
extern struct proc_st *proc;

/* search from inode m_dir ,to find a dir_entry named 'entry_name' */
BUFFER_HEAD *find_entry (MEM_INODE **m_dir,i8 *entry_name,
                        u32 name_len,DIR_ENTRY **bh_de_ptr) 
    /* why **dir ,not *dir ?
     * just for fake dir .. cross when approach the mounted inode 
     * to bring the new inode back to the caller !
     */
{
    MEM_SUPER_BLOCK *sb;

    if (!m_dir || !(*m_dir) || !bh_de_ptr || !(*m_dir)->i_dev) 
        return (NULL);
    if (!name_len) 
        return ((void*)((*bh_de_ptr) = NULL));
    if (name_len > (NAME_LEN)) 
        name_len = NAME_LEN - 1;            /* truncate */
    BUFFER_HEAD *bh;
    i32 src_len,i,j,blk_no,blk_nr = ((*m_dir)->i_size + BLK_SIZE - 1) >> 12;
    DIR_ENTRY *de_ptr = NULL;

    if (!strncmp (entry_name,"..",2)) {
        if ((*m_dir)->i_num == ROOT_IND_NR) {   /* swap fs */
            sb = get_sblk ((*m_dir)->i_dev);
            if (sb->sb_i_mounted) {
                iput (*m_dir);
                *m_dir = sb->sb_i_mounted;
                (*m_dir)->i_cnt ++;
            }
        } else if (((*m_dir)->i_dev == proc->root.dev) && ((*m_dir)->i_num == proc->root.inr)) {
            name_len = 1;           /* when chroot ,this is needed */
        } else ;
    }
    for (i = 0 ; i < (NR_BLK_OBJS * NR_BLK_OBJS + NR_BLK_OBJS + 7)  && blk_nr > 0; i ++) {
        if (!(blk_no = bmap ((*m_dir),i))) 
            continue ; /* next block */
        bh = bread ((*m_dir)->i_dev,blk_no);
        blk_nr --;
        if (!bh) 
            continue;
        de_ptr = (DIR_ENTRY*)(bh->bh_buf);      
        for (j = 0 ; j < ENTRIES_PER_BLK ; j ++) {
            if (IND_EMPTY(de_ptr[j].inode_nr) && !de_ptr[j].name[0]) 
                continue ;
            src_len = strlen (de_ptr[j].name);
            if  (name_len > src_len)
                src_len = name_len;
            if (!strncmp (de_ptr[j].name,entry_name,src_len)) {
                (*bh_de_ptr) = &de_ptr[j];  /* bh_de_ptr is buffer's dir_entry ptr */
                return (bh);
            }
        }
        brelse (bh);
    }
    return (NULL);
}

/* add a entry to m_dir */
BUFFER_HEAD *add_entry (MEM_INODE *m_dir,i8 *entry_name,
                        u32 name_len,DIR_ENTRY **bh_de_ptr) 
{
    if (!m_dir || !bh_de_ptr || !m_dir->i_dev) 
        return (NULL);
    if (!name_len) 
        return ((void*)((*bh_de_ptr) = NULL));
    if (name_len > (NAME_LEN)) 
        name_len = NAME_LEN - 1;            /* truncate */
    BUFFER_HEAD *bh;
    i32 i,j,blk_no;
    DIR_ENTRY *de_ptr = NULL;
    u8 crted = true;

    for (i = 0 ; i < (NR_BLK_OBJS * NR_BLK_OBJS + NR_BLK_OBJS + 7); i ++) {
        if (!(blk_no = create_block (m_dir,i,&crted))) 
            return (NULL) ;
        if (crted == 0xFF) {
            m_dir->i_size += BLK_SIZE;      /* new block */
            m_dir->i_crt_time = CUR_TIME;
            m_dir->i_dirt = true;
        }
        if (!(bh = bread (m_dir->i_dev,blk_no))) 
            continue;  
        de_ptr = (DIR_ENTRY*)(bh->bh_buf);      
        for (j = 0 ; j < ENTRIES_PER_BLK ; j ++) {
            if (IND_EMPTY (de_ptr[j].inode_nr) && !de_ptr[j].name[0]) {
                m_dir->i_mtime = CUR_TIME;
                m_dir->i_dirt = true;
                strncpy (de_ptr[j].name,entry_name,name_len);
                de_ptr[j].name[name_len] = '\0';
                (*bh_de_ptr) = &de_ptr[j];
                bh->bh_dirt = true;
                return (bh);
            }
        }
        brelse (bh);
    }
    return (NULL);
}

extern  MEM_INODE *iget (u16,u32);
extern  void iput (MEM_INODE *);

/* get the last dir inode */
LOCAL MEM_INODE *get_last_dir (RO i8 *path)
{
    if (!path) return (NULL);

    MEM_INODE *m_dir;
    i8 *p = (i8*)path,c ;
    BUFFER_HEAD *bh;
    DIR_ENTRY *bh_de_ptr ;
    u16 dev_no ;
    u32 inode_nr,name_len = 0;

    if (path[0] == '/' ) { /* root */
        m_dir = iget (proc->root.dev,proc->root.inr);
        path ++ ;
    } else             /* cwd */
        m_dir = iget (proc->cwd.dev,proc->cwd.inr);
    if (!m_dir) 
        return (NULL);
    while (true) {
        p = (i8*)path;
        if (!S_ISDIR(m_dir->i_mode)) {
            iput (m_dir);
            break;
        }
        /* get name len from path */
        for (name_len = 0;(c = *path ++ ) && (c != '/'); name_len ++);
        if (!c)  
            return (m_dir);
        if (!(bh = find_entry (&m_dir,p,name_len,&bh_de_ptr))) { /* can not find */
            iput (m_dir);
            break;
        }
        inode_nr = bh_de_ptr->inode_nr;
        dev_no = m_dir->i_dev;
        iput (m_dir);
        brelse (bh);
        if (!(m_dir = iget (dev_no,inode_nr)))
            break;
    }
    return (NULL);
}  

LOCAL MEM_INODE *dir_namei (RO i8 *path,u32 *name_len,RO i8**name) 
{
    MEM_INODE *m_dir;
    RO i8 *base_name;
    i8 ch;

    if (!(m_dir = get_last_dir (path)))
        return (NULL);
    base_name = path;
    while ((ch = *path ++)) 
        if ( '/' == ch ) 
            base_name = path;
    *name_len = path - base_name - 1;    /* sub a '/' */
    *name = base_name ;
    return (m_dir);
}

MEM_INODE *namei (RO i8 *path) 
{
    RO i8 *base;
    u16 dev_no;
    u32 inode_nr,name_len = 0;
    DIR_ENTRY *bh_de_ptr;
    BUFFER_HEAD *bh;
    MEM_INODE *m_dir;

    if (!(m_dir = dir_namei (path,&name_len,&base))) 
        return (NULL);
    if (!name_len) 
        return (m_dir);
    if (!(bh = find_entry (&m_dir,(i8*)base,name_len,&bh_de_ptr)))  {
        iput (m_dir);
        return (NULL);
    }
    inode_nr = bh_de_ptr->inode_nr;
    dev_no = m_dir->i_dev;
    brelse (bh);
    iput (m_dir);
    if ((m_dir = iget (dev_no,inode_nr))) {
        m_dir->i_acc_time = CUR_TIME;
        //m_dir->i_dirt = true; /* yes it's clean */
    }
    return (m_dir);
}

extern int crash (RO i8 *fmt,...);
extern MEM_INODE *new_inode (u16);
extern u32 new_block (u16);
extern void free_block (u16,u32);

/* make a dir */
i32 do_mkdir (RO i8 *path ,u16 mode) 
{
    RO i8 *base;
    u32 namelen ;
    MEM_INODE *m_prev_dir,*m_new_dir;
    BUFFER_HEAD *bh,*new_dir_block ;
    DIR_ENTRY *bh_de_ptr;

    if (!(m_prev_dir = dir_namei (path,&namelen,&base)))
        return (-ENOPATH);
    if (!namelen) {
        iput (m_prev_dir);
        return (-ENOPATH);
    }
    /* already exist */
    if ((bh = find_entry (&m_prev_dir,(i8*)base,namelen,&bh_de_ptr))) {
        brelse (bh);
        iput (m_prev_dir);
        return (-EPDUP);
    }
    if (!(m_new_dir = new_inode (m_prev_dir->i_dev))) {
        iput (m_prev_dir);
        return (-ENOSRC);
    }
    if (!(m_new_dir->i_zone[0] = new_block (m_new_dir->i_dev))) {
        iput (m_prev_dir);
        m_new_dir->i_nlinks = 0;
        iput (m_new_dir);
        return (-ENOSRC);
    }
    m_new_dir->i_size = BLK_SIZE;
    m_new_dir->i_mtime = m_new_dir->i_acc_time = CUR_TIME;
    m_new_dir->i_dirt = true;
    if (!(new_dir_block = bread (m_new_dir->i_dev,m_new_dir->i_zone[0]))) {
        iput (m_prev_dir);
        free_block (m_new_dir->i_dev,m_new_dir->i_zone[0]);
        m_new_dir->i_nlinks = 0 ;
        iput (m_new_dir);
        return (-EBADRD);
    }
    bh_de_ptr = (DIR_ENTRY*)(new_dir_block->bh_buf);
    bh_de_ptr[0].inode_nr = m_new_dir->i_num;
    bh_de_ptr[0].name[0] = '.';
    bh_de_ptr[0].name[1] = '\0';            /* "." */

    bh_de_ptr[1].inode_nr = m_prev_dir->i_num    ;//prev inode nr 
    bh_de_ptr[1].name[0] = '.';
    bh_de_ptr[1].name[1] = '.';
    bh_de_ptr[1].name[2] = '\0';

    m_new_dir->i_nlinks = 2;  /* name links == 2,1 for '.' 1 for self name dir */
    new_dir_block->bh_dirt = true;
    brelse (new_dir_block);

    m_new_dir->i_mode = I_DIRECTORY;
    m_new_dir->i_uid = UID;
    m_new_dir->i_gid = GID;
    m_new_dir->i_magic = I_MAGIC;
    m_new_dir->i_dirt = true;
    m_new_dir->i_valid = true;
    /* now every thing is ok ,start to add a entry at 
     * prev dir blocks 
     */
    if  (!(bh = add_entry (m_prev_dir,(i8*)base,namelen,&bh_de_ptr))) {
        iput (m_prev_dir);
        free_block (m_new_dir->i_dev,m_new_dir->i_zone[0]);
        m_new_dir->i_nlinks = 0;
        iput (m_new_dir);
        return (-ENOSRC);
    }
    bh_de_ptr->inode_nr = m_new_dir->i_num;
    bh->bh_dirt = true;

    m_prev_dir->i_nlinks ++;    /* add one for new dir */
    m_prev_dir->i_dirt = true;
    m_prev_dir->i_valid = true;
    iput (m_prev_dir);
    iput (m_new_dir);
    brelse (bh);
    return (0);
}

LOCAL u8 is_dir_empty (MEM_INODE *m_dir)
{
    if (!m_dir || !m_dir->i_dev) 
        return (true);
    BUFFER_HEAD *bh;
    i32 i,j,blk_no,blk_nr = (m_dir->i_size + BLK_SIZE - 1) >> 12;
    DIR_ENTRY *de_ptr = NULL;

    for (i = 0 ; i < (NR_BLK_OBJS * NR_BLK_OBJS + NR_BLK_OBJS + 7)  && blk_nr > 0; i ++) {
        if (!(blk_no = bmap (m_dir,i))) 
            continue ; /* next block */
        bh = bread (m_dir->i_dev,blk_no);
        blk_nr --;
        if (!bh) continue;
        de_ptr = (DIR_ENTRY*)(bh->bh_buf);      
        for (j = 2 ; j < ENTRIES_PER_BLK ; j ++) 
            if (de_ptr[j].name[0]) 
                return (false);
        brelse (bh);
    }
    return (true);
}

LOCAL u8 list_dir (MEM_INODE **m_dir)
{
    if (!m_dir || !(*m_dir) || !(*m_dir)->i_dev) 
        return (0);
    BUFFER_HEAD *bh;
    i32 i,j,blk_no,blk_nr = ((*m_dir)->i_size + BLK_SIZE - 1) >> 12;
    DIR_ENTRY *de_ptr = NULL;
    //MEM_INODE *m_inode;

    for (i = 0 ; i < (NR_BLK_OBJS * NR_BLK_OBJS + NR_BLK_OBJS + 7)  && blk_nr > 0; i ++) {
        if (!(blk_no = bmap ((*m_dir),i))) 
            continue ; /* next block */
        bh = bread ((*m_dir)->i_dev,blk_no);
        blk_nr --;
        if (!bh) 
            continue;
        de_ptr = (DIR_ENTRY*)(bh->bh_buf);    
        for (j = 0; j < ENTRIES_PER_BLK ; j ++) { 
            if (de_ptr[j].name[0]) {
                //if ((m_inode = iget ((*m_dir)->i_dev,de_ptr[j].inode_nr))) {
                  //  if (S_ISDIR (m_inode->i_mode))
                    //    printf ( "%s/ ",de_ptr[j].name);
                    //else
                        printf ( "%s  ",de_ptr[j].name );
                    //iput (m_inode);
               // }
            }
        }
        brelse (bh);
    }
    printf ( "\n" );
    return (1);
}

extern void warn (const char *fmt,...);
/* remove a directory */
i32 do_rmdir (RO i8 *path) 
{
    MEM_INODE *m_prev_dir,*m_this_dir;
    u32 name_len = 0;
    RO i8 *base ;
    BUFFER_HEAD *bh;
    DIR_ENTRY *bh_de_ptr;

    if (!(m_prev_dir = dir_namei (path,&name_len,&base))) 
        return (-ENOPATH);
    if (!name_len) {
        iput (m_prev_dir);
        return (-ENOPATH);
    }
    if (!(bh = find_entry (&m_prev_dir,(i8*)base,name_len,&bh_de_ptr))) {
        iput (m_prev_dir);
        return (-ENOPATH);
    }
    if (!(m_this_dir = iget (m_prev_dir->i_dev,bh_de_ptr->inode_nr))){ 
        iput (m_prev_dir);
        brelse (bh);
        return (-ENOPATH);
    }
    if (m_prev_dir == m_this_dir) { /* self-kill is not allowed ! */
        iput (m_prev_dir);
        iput (m_this_dir);
        brelse (bh);
        return (-ENOPERM);
    }
    if (m_this_dir->i_dev != m_prev_dir->i_dev || m_this_dir->i_cnt > 1) { /* diff dev */
        warn ( "inr cnt %d > 1.",m_this_dir->i_cnt );
        iput (m_prev_dir);
        iput (m_this_dir);
        brelse (bh);
        return (-EOBUSY);
    }
    if (m_this_dir->i_num == ROOT_IND_NR) { /* root-kill is not allowed also */
        iput (m_prev_dir);
        iput (m_this_dir);
        brelse (bh);
        return (-ENOPERM);
    }
    if (!S_ISDIR (m_this_dir->i_mode)) {    /* dir ? */
        iput (m_prev_dir);
        iput (m_this_dir);
        brelse (bh);
        return (-ENOTD);
    }
    if (!is_dir_empty (m_this_dir)) { /* empty ? */
        iput (m_prev_dir);
        iput (m_this_dir);
        brelse (bh);
        return (-ENOTEMPT);
     }
     if (m_this_dir->i_nlinks != 2) {
        warn ( "rm dir links is %d ",m_this_dir->i_nlinks ) ;
        iput (m_prev_dir);
        iput (m_this_dir);
        brelse (bh);
    }
    bh_de_ptr->inode_nr = 0;
    memset (bh_de_ptr->name,0,NAME_LEN);
    bh->bh_dirt = true;
    brelse (bh);

    m_this_dir->i_nlinks = 0;
    m_this_dir->i_dirt  = false;
    /* this can not be dirt 'cause it is to be removed 
     * no need to write back to disk 
     */
    m_prev_dir->i_nlinks -- ;
    m_prev_dir->i_mtime = m_this_dir->i_acc_time = CUR_TIME;
    m_prev_dir->i_dirt = true;  /* this is dir inode need not to 
                                 * write back ,however prev parent 
                                 * dir inode need to do this 
                                 */
    iput (m_prev_dir);
    iput (m_this_dir);
    return (0);
} 

i32 do_ls (RO i8 *path)
{
    if (!path)
        return (-ENOPATH);
    MEM_INODE *m_prev_dir;

    if (!(m_prev_dir = namei (path))) 
        return (-ENOPATH);
    if (!S_ISDIR (m_prev_dir->i_mode)) {
        iput (m_prev_dir);
        return (-ENOTD);
    }
    list_dir (&m_prev_dir);
    iput (m_prev_dir);
    return (0);
}

i32 do_chdir (RO i8 *path)
{
    MEM_INODE *m_inode;

    if (!(m_inode = namei (path))) 
        return (-ENOPATH);
    if (!S_ISDIR(m_inode->i_mode)) {
        iput (m_inode);
        return (-ENOTD);
    }
    proc->cwd.inr = m_inode->i_num;
    proc->cwd.dev  = m_inode->i_dev;
    iput (m_inode);
    return (proc->cwd.inr);
}

i32 do_chroot (RO i8 *path)
{
    MEM_INODE *m_inode;

    if (!(m_inode = namei (path))) 
        return (-ENOPATH);
    if (!S_ISDIR(m_inode->i_mode)) {
        iput (m_inode);
        return (-ENOTD);
    }
    proc->root.inr = m_inode->i_num;
    proc->root.dev  = m_inode->i_dev;
    iput (m_inode);
    return (proc->root.inr);
}

i32 do_chmod (RO i8 *path,u16 mode)
{
    return (0);
}

extern void clear_iblks (MEM_INODE *m_inode) ;

/* open inode */
i32 open_namei (RO i8 *path,u16 flag,u16 mode,MEM_INODE **dest_inode)
{
    RO i8 *base ;
    u32 inode_nr = 0,name_len;
    u16 dev_no;
    MEM_INODE *m_prev_dir,*m_new_inode ;
    BUFFER_HEAD *bh;
    DIR_ENTRY *bh_de_ptr;

    if ((flag & O_RDONLY) && !(flag&O_ACCMODE))   
        flag |= O_WRONLY;
    mode |= I_REGULAR;  /* regular file type */
    if (!(m_prev_dir = dir_namei (path,&name_len,&base))) 
        return (-ENOPATH);
    if (!name_len) {    /* no name after '/' */
        if (!(flag & (O_ACCMODE|O_CREAT|O_TRUNC))) {   
            (*dest_inode) = m_prev_dir;
            return (0);
        }
        iput (m_prev_dir);
        return (-ENOPATH);
    }
    if (!(bh = find_entry (&m_prev_dir,(i8*)base,name_len,&bh_de_ptr))) {
        if (!(flag & O_CREAT)) {    /* not create file */
            iput (m_prev_dir);
            return (-ENOPATH);
        }
        if (!(m_new_inode = new_inode (m_prev_dir->i_dev))) {   /* create */
            iput (m_prev_dir);
            return (-ENOSRC);
        }
        m_new_inode->i_uid      = UID;
        m_new_inode->i_gid      = GID;
        m_new_inode->i_mode     = mode;
        m_new_inode->i_dirt     = true;
        m_new_inode->i_valid    = true;

        if (!(bh = add_entry (m_prev_dir,(i8*)base,name_len,&bh_de_ptr))) {
            m_new_inode->i_nlinks = 0;
            iput (m_new_inode);
            iput (m_prev_dir);
            return (-ENOSRC);
        }
        bh_de_ptr->inode_nr = m_new_inode->i_num;
        bh->bh_dirt = true; /* dirt */
        brelse (bh);
        iput (m_prev_dir);
        (*dest_inode) = m_new_inode;   /* return new inode */ 
        return (0);         
    }
    /* found one ,then open it with mode & flags 
     * it must be regular file ....
     */
    inode_nr = bh_de_ptr->inode_nr;
    dev_no = m_prev_dir->i_dev;
    iput (m_prev_dir);
    brelse (bh);    
    if (flag & O_EXCL) 
        return (-1);
    if (!(m_new_inode = iget (dev_no,inode_nr)))
        return (-1);
    if (S_ISDIR (m_new_inode->i_mode) && (flag & O_ACCMODE)){ /* denied or is dir mode */
        iput (m_new_inode);
        return (-EDIR);
    }
    m_new_inode->i_acc_time = CUR_TIME;
    if (flag & O_TRUNC)                 /* truncate size 0 */
        clear_iblks (m_new_inode);
    *dest_inode = m_new_inode ;
    return (0);
}

/* create a hard link rename */
i32 do_hlink (RO i8 *old_name,RO i8 *new_name) 
{
    DIR_ENTRY *de;
    MEM_INODE *m_old_inode,*m_new_prev_dir ;
    RO i8 *base;
    u32 namelen = 0;
    BUFFER_HEAD *bh;

    if (!(m_old_inode = namei (old_name))) 
        return (-ENOPATH);
    if (S_ISDIR (m_old_inode->i_mode)) { /* dir mode */
        iput (m_old_inode);
        return (-EDIR);
    }
    if (!(m_new_prev_dir = dir_namei (new_name,&namelen,&base))){ /* get new name dir inode */
        iput (m_old_inode);
        return (-ENOPATH);
    }
    if (!namelen) {
        iput (m_old_inode);
        iput (m_new_prev_dir);
        return (-ENOPATH);
    }
    if ((m_old_inode->i_dev != m_new_prev_dir->i_dev)) {
        warn ( "bad dev %x != %x \n",m_old_inode->i_dev,m_new_prev_dir->i_dev );
        iput (m_old_inode);
        iput (m_new_prev_dir);
        return (-ENOPERM);
    }
    if ((bh = find_entry (&m_new_prev_dir,(i8*)base,namelen,&de))) {
        brelse (bh);
        iput (m_old_inode);
        iput (m_new_prev_dir);
        return (-EPDUP);
    }
    if (!(bh = add_entry (m_new_prev_dir,(i8*)base,namelen,&de))) {
        iput (m_old_inode);
        iput (m_new_prev_dir);
        return (-ENOSRC);
    }
    de->inode_nr = m_old_inode->i_num;  /* */
    bh->bh_dirt = true;
    brelse (bh);
    iput (m_new_prev_dir);
    /* old name inode links ++ */
    m_old_inode->i_nlinks ++;
    m_old_inode->i_acc_time = CUR_TIME;
    m_old_inode->i_dirt = true;
    iput (m_old_inode);
    return (0);
}

/* rm a hard link for inode */
i32 do_unhlink (RO i8* name) 
{
    RO i8 *base;
    u32 namelen ;
    MEM_INODE *m_prev_dir,*m_this_inode;
    BUFFER_HEAD *bh;
    DIR_ENTRY *bh_de_ptr;

    if (!(m_prev_dir = dir_namei (name,&namelen,&base))) 
        return (-ENOPATH);
    if (!namelen) {
        iput (m_prev_dir);
        return (-ENOPATH);
    }
    if (!(bh = find_entry (&m_prev_dir,(i8*)base,namelen,&bh_de_ptr))) {
        iput (m_prev_dir);
        return (-ENOPATH);
    }
    if (!(m_this_inode = iget (m_prev_dir->i_dev,bh_de_ptr->inode_nr))) {
        iput (m_prev_dir);
        brelse (bh);
        return (-ENOPATH);
    }
    if (S_ISDIR (m_this_inode->i_mode)) { 
        iput (m_this_inode);
        iput (m_prev_dir);
        brelse (bh);
        return (-EDIR);
    }
    if (!m_this_inode->i_nlinks) {
        warn ( "del non-existent file (dev 0x%x,inr %d) with links = 0 !",
                m_this_inode->i_dev,m_this_inode->i_num );
        m_this_inode->i_nlinks = 1; /* fall through */
    }
    bh_de_ptr->inode_nr = 0;
    memset (bh_de_ptr->name,0,NAME_LEN);
    bh->bh_dirt = true;
    brelse (bh);
    m_this_inode->i_nlinks --;
    m_this_inode->i_dirt = true;
    m_this_inode->i_mtime  = CUR_TIME;
    iput (m_this_inode);
    iput (m_prev_dir);
    return (0);
}

/* make inode */
i32 do_mknod (RO i8* path,u16 mode,u16 dev)
{
    RO i8 *basename;    
    u32 namelen = 0;
    MEM_INODE *m_dir,*m_inode;
    BUFFER_HEAD *bh;
    DIR_ENTRY *de;

    if (!(m_dir = dir_namei (path,&namelen,&basename)))
        return (-ENOPATH);
    if (!namelen) {
        iput (m_dir);
        return (-ENOPATH);
    }
    if ((bh = find_entry (&m_dir,(i8*)basename,namelen,&de))) {
        brelse (bh);
        iput (m_dir);
        return (-EPDUP);
    }
    if (!(m_inode = new_inode (m_dir->i_dev))) {
        iput (m_dir);
        return (-ENOSRC);
    }
    m_inode->i_mode = mode;
    if (S_ISBLK (mode) || S_ISCHR (mode))
        m_inode->i_zone[0] = dev;           /* devno in special dev file-> zone[0] */
    m_inode->i_mtime = m_inode->i_acc_time = CUR_TIME;
    m_inode->i_dirt = true;

    if (!(bh = add_entry (m_dir,(i8*)basename,namelen,&de))) {
        iput (m_dir);
        m_inode->i_nlinks = 0;  /* clean up */
        iput (m_inode);
        return (-ENOSRC);
    }
    de->inode_nr = m_inode->i_num;
    bh->bh_dirt = true;
    iput (m_dir);
    iput (m_inode);
    brelse (bh);
    return (0);
}

LOCAL void idump (MEM_INODE *p)
{
    i32 i  = 0;

	if (p) {
		printf ( "inode->mode   = 0x%-8x \n",p->i_mode );
		printf ( "inode->uid    = 0x%-8x \n",p->i_uid );
		printf ( "inode->gid    = 0x%-8x \n",p->i_gid );
		printf ( "inode->size   = 0x%-8x \n",p->i_size );
		printf ( "inode->nlinks = 0x%-8x \n",p->i_nlinks );
		printf ( "inode->i_cnt  = 0x%-8x \n",p->i_cnt );
		printf ( "inode->i_magic= 0x%-8x \n",p->i_magic );
        for (i = 0;i < 9 ; i++) 
		    printf ( "inode->i_zone[%d]= 0x%-8x \n",i,p->i_zone[i] );
	}
}

i32 do_dumpnod (RO i8 *path)
{
    MEM_INODE *m_dir;

    if (!(m_dir = namei (path)))
        return (-ENOPATH);
    if (S_ISREG (m_dir->i_mode)) {
        idump (m_dir);
        iput (m_dir);
        return (0);
    } else if (S_ISBLK (m_dir->i_mode)) {
        printf ( "dev_no = 0x%x \n",m_dir->i_zone[0] );
        iput (m_dir);
        return (0);
    } else {
        iput (m_dir);
        return (-ENOPERM);
    }
    return (0);
}

i32 pushd (RO i8 *path)
{
    MEM_INODE *m_inode;

    if (!path || !(m_inode = namei (path))) 
        return (-ENOPATH);
    if (!S_ISDIR (m_inode->i_mode)) {
        iput (m_inode);
        return (-ENOTD);
    }
    if ((strlen (proc->pwd) + strlen (path)) >= PWD_LEN) {
        iput (m_inode);
        return (-EOISFULL);
    }
    strcat (proc->pwd,path);
    iput (m_inode);
    return (0);
}

i8 *popd (void)
{
    return (proc->pwd);
}

i8 *setd (RO i8 *path)
{
    return (proc->pwd);
}

i8 *getpwd (void) 
{
    return (proc->pwd);
}
