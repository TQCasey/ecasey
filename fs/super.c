#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <ecasey/fs.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

MEM_SUPER_BLOCK super_blk [NR_SBLKS];

extern BUFFER_HEAD *get_blk (u16 dev_no,u32 blk_no) ;
extern void brelse (BUFFER_HEAD *bh_free);
extern BUFFER_HEAD *bread (u16 dev_no,u32 blk_no) ;
extern int crash (const char *fmt,...) ;
extern void warn (const char *fmt,...);
extern MEM_INODE* iget (u16 dev_no,u32 inode_nr);
extern void iput (MEM_INODE *m_puti) ;

/* read ibmaps */
LOCAL void read_ibmap (MEM_SUPER_BLOCK *sb) 
{
    u32 i,blk_nr = 2;
    BUFFER_HEAD *tmp;

    for (i = 0 ; i < sb->sb_imap_blocks ; i ++ ,blk_nr ++) {
        if (!(tmp = bread (sb->sb_dev,blk_nr)))    
            crash ( "read imap failed!\n" );
        sb->sb_imap_bh[i] = tmp; 
    }
    for (i = 0 ; i < sb->sb_zmap_blocks ; i ++ ,blk_nr ++) {
        if (!(tmp = bread (sb->sb_dev,blk_nr)))    
            crash ( "read bmap failed!\n" );
        sb->sb_zmap_bh[i] = tmp;
    }
}

MEM_SUPER_BLOCK* get_sblk (u16 dev_no) 
{
	i32 i = 0 ;

    if (!dev_no) return (NULL); 
    for (i = 0; i < NR_SBLKS ; i ++)
        if (super_blk[i].sb_dev == dev_no) 
            return (super_blk+i);           /* cached ? */
    for (i = 0 ; i < NR_SBLKS ; i ++) {
        if (!super_blk[i].sb_dev) {
            BUFFER_HEAD *tmp = bread (dev_no,1);    /* validate,not dirt  */
            if (!tmp) 
                return (NULL);
            memcpy (super_blk+i,tmp->bh_buf,sizeof(DEV_SUPER_BLOCK));
            super_blk [i].sb_dev = dev_no;
            brelse (tmp);                           /* free buffer */
            return (super_blk+i);   
        }
    }
    warn ( "more than %d fs mounted !\n",NR_SBLKS );
    return (NULL);
}

void _put_sblk (u8 root,u16 dev_no) 
{
    MEM_SUPER_BLOCK *sblk = NULL;
	i32 i = 0 ;

    if (root && (dev_no == ROOT_DEV)) {
        warn ( "put rootfs !" );
        return ;
    }
    if (!dev_no || !(sblk = get_sblk(dev_no))) 
        return ;
    if (sblk->sb_i_mounted) {
        warn ( "put mounted dev !" );
        return ;
    }
    if (sblk->sb_dirt) {        /* sync to buffer  */  
        BUFFER_HEAD *tmp = get_blk (dev_no,1);
        if (!tmp) 
            crash ( "get block failed !" );
        memcpy (tmp->bh_buf,sblk,sizeof(DEV_SUPER_BLOCK));
        tmp->bh_dirt = true;
        tmp->bh_valid = true;
        brelse (tmp);           
    }
    for (i = 0 ; i < sblk->sb_zmap_blocks; i ++)
        brelse (sblk->sb_zmap_bh[i]);   
    for (i = 0 ; i < sblk->sb_imap_blocks ;i ++)   
        brelse (sblk->sb_imap_bh[i]);   /* sync i/b map */
    sblk->sb_dev = 0;
    memset (sblk,0,sizeof(sblk));
}

INLINE void put_rootfs (u16 dev)
{
    _put_sblk (0,dev);
}

INLINE void put_sblk (u16 dev)
{
    _put_sblk (1,dev);
}

extern i32 free_inode (MEM_INODE *inode) ;
extern MEM_INODE* new_inode (u16 dev_no) ;
extern u32 new_block (u16 dev_no) ;
extern void free_block (u16 dev_no,u32 blk_nr) ;

void creat_rooti (MEM_SUPER_BLOCK *sb)
{
    MEM_INODE *root = new_inode (sb->sb_dev);   /* new for root inode */
    if (!(root->i_zone[0] = new_block (root->i_dev)))
        crash ("can not create basic rooti");
    root->i_size    = BLK_SIZE;
    root->i_mtime   = root->i_acc_time = CUR_TIME;
    root->i_mode    = I_DIRECTORY;
    root->i_uid     = UID;
    root->i_gid     = GID;
    root->i_dirt    = true;
    root->i_valid   = true;

    BUFFER_HEAD *bh;
    DIR_ENTRY *bh_de_ptr;

    if ( !(bh = bread (root->i_dev,root->i_zone[0])) ) 
        crash ( "can not read root entries block !\n" );
    bh_de_ptr = (DIR_ENTRY*)(bh->bh_buf);
    bh_de_ptr[0].inode_nr   = ROOT_IND_NR;
    bh_de_ptr[0].name[0]    = '.';
    bh_de_ptr[0].name[1]    = '\0';

    bh_de_ptr[1].inode_nr   = ROOT_IND_NR;
    bh_de_ptr[1].name[0]    = '.';
    bh_de_ptr[1].name[1]    = '.';
    bh_de_ptr[1].name[2]    = '\0';

    root->i_nlinks = 2;
    bh->bh_dirt = true;
    brelse (bh);
    iput (root);
}

LOCAL void format_rootfs (MEM_SUPER_BLOCK *sb)
{
    u32 i = 0 , blk_nr = 2;
	BUFFER_HEAD *tmp;

    printf ( "Formating ROOTFS ...\n" );
    sb->sb_ninodes = NR_INODES ;                  
    sb->sb_nzones  = NR_ZONES;               
    sb->sb_zmap_blocks = NR_ZMAP_BLKS;
    sb->sb_imap_blocks = NR_IMAP_BLKS;
    sb->sb_first_datazone = 2 + NR_IMAP_BLKS + NR_ZMAP_BLKS + 
        /* boots + sblk + imap + zmap*/
        ((NR_INODES * sizeof(DEV_INODE)>>BLK_SIZE_SFT)); /* inode_table */  
    sb->sb_log_zone_size = 0;    
    sb->sb_max_size = NR_ZMAP_BLKS * (BLK_SIZE << 3) * BLK_SIZE;/* max 1G size */
    sb->sb_magic = CASEY_MAGIC;
    sb->sb_pos  = 0;

    if (!(tmp = bread (sb->sb_dev,1)))
        crash ("setup sblk failed at getblk !" );
    memset (tmp->bh_buf,0,BLK_SIZE);
    memcpy (tmp->bh_buf,(void*)sb,sizeof(DEV_SUPER_BLOCK));
    tmp->bh_dirt = true;
    brelse (tmp);                                           /* sync later */

    for (i = 0 ; i < sb->sb_imap_blocks ; i ++ ,blk_nr ++) {
        if (!(tmp = bread (sb->sb_dev,blk_nr)))    
            crash ( "read imap failed!" );
        memset (tmp->bh_buf,0,BLK_SIZE);
        tmp->bh_dirt = true;
        sb->sb_imap_bh[i] = tmp; 
    }
    for (i = 0 ; i < sb->sb_zmap_blocks ; i ++ ,blk_nr ++) {
        if (!(tmp = bread (sb->sb_dev,blk_nr)))    
            crash ( "read bmap failed!" );
        memset (tmp->bh_buf,0,BLK_SIZE);
        tmp->bh_dirt = true;
        sb->sb_zmap_bh[i] = tmp;
    }
    creat_rooti (sb);
    printf ( "ROOTFS Formated Done .\n" );
}

void load_rootfs (u16 dev_no)
{
    MEM_SUPER_BLOCK *sb = get_sblk (dev_no);
    if (!sb) 
        crash ("read super block failed !\n" );
    if (sb->sb_magic != CASEY_MAGIC) 
        format_rootfs (sb);
    else 
        read_ibmap (sb);
}

extern MEM_INODE *namei (RO i8 *path) ;

i32 do_mount (RO i8 *dev_name,RO i8 *dir_name,u32 rw)
{
    MEM_INODE *m_dir;
    u16 dev;

    if (!(m_dir = namei (dev_name)))
        return (-ENOPATH);
    if (!S_ISBLK (m_dir->i_mode)) {
        iput (m_dir);
        return (-ENOPERM);
    }
    dev = m_dir->i_zone [0];
    iput (m_dir);

    if (!(m_dir = namei (dir_name)))
        return (-ENOPATH);
    if (m_dir->i_cnt != 1 || m_dir->i_num == ROOT_IND_NR) {
        iput (m_dir);   /* root re-mounted or quato not ONLY at here is not allowed */
        return (-EOBUSY);
    }
    if (!S_ISDIR (m_dir->i_mode)) {
        iput (m_dir);
        return (-ENOTD);
    }
    MEM_SUPER_BLOCK *sb = get_sblk (dev);
    if (!sb) {
        iput (m_dir);
        return (-EOINVALID);
    }
    if (sb->sb_magic != CASEY_MAGIC) {
        iput (m_dir);
        sb->sb_dev = 0;
        return (-EOINVALID);
    }
    if (sb->sb_i_mounted) {
        iput (m_dir);
        return (-ENOPERM);
    }
    /* time to read i/b map */
    read_ibmap (sb) ;

    sb->sb_i_mounted = m_dir;   /* m_dir is the host dir_inode */
    sb->sb_ro = (rw != 0);
    m_dir->i_mounted = true;
    m_dir->i_dirt   = true;
    return (0);
}

extern struct proc_st *proc;
extern MEM_INODE inode_table[NR_INODE_BUF] ; 
extern void sync_dev_blks (u16 dev_no);
extern void sync_dev_inodes (void);

i32 do_umount (RO i8 *dir_name)
{
    MEM_INODE *m_dir;
    u16 dev;
    MEM_INODE *cwd;
    u32 i = 0;

    if (!(m_dir = namei (dir_name)))
        return (-ENOPATH);
    if (!S_ISDIR (m_dir->i_mode)) {
        iput (m_dir);
        return (-ENOTD);    /* m_dir is slaver dir_inode */
    }
    dev = m_dir->i_dev;
    iput (m_dir);

    if (ROOT_DEV == dev)   /* root-killed not allowed */
        return (-EOBUSY);   /* no need to iput (m_dir) */

    MEM_SUPER_BLOCK *sb ;
    
    if (!(sb = get_sblk (dev)) || !sb->sb_i_mounted)   /* non-mounted dev-kill */
        return (-ENOPERM);
    if (!sb->sb_i_mounted->i_mounted) 
        warn ( "mounted inode has i_mounted = 0 \n");
    /* xfs don't use inode ptr but inode_nr,which the inode->cnt may be zero
     * and we can not umount cwd dir,so we get cwd inode first !
     */
    cwd = iget (proc->cwd.dev,proc->cwd.inr);

    for (i = 0 ; i < NR_INODE_BUF ; i ++) {
        if (inode_table [i].i_dev == dev && inode_table [i].i_cnt) {
            iput (cwd);
            return (-EOBUSY);
        }
    }
    /* all safe to kill inode */
    sb->sb_i_mounted->i_mounted = false;
    iput (sb->sb_i_mounted);    /* iput host dir inode */
    sb->sb_i_mounted = NULL;
    iput (cwd);             /* why just chk the inode_tbls 
                             * what if buffer hash is not free .bug here ???
                             * some one can help me ?
                             */
    put_sblk (dev);
    sync_dev_inodes ();
    sync_dev_blks (dev);    
    return (0);
}
