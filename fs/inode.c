#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <ecasey/fs.h>
#include <sys/stat.h>

MEM_INODE inode_table[NR_INODE_BUF] ; 

extern MEM_SUPER_BLOCK *get_sblk (u16 dev_no);
extern int crash (const char *fmt,...);
extern BUFFER_HEAD* bread (u16,u32);
extern void brelse (BUFFER_HEAD *);
extern void warn (const char *fmt,...);

/* reading inode from dev without chking inr */
LOCAL void read_dev_inode (MEM_INODE *m_inode) 
{
    u32 inode_blk_nr,inode_bit_nr;
    MEM_SUPER_BLOCK *sb;
	BUFFER_HEAD *bh;

    if (!m_inode || !m_inode->i_dev || m_inode->i_valid) 
        return ;
    if (!(sb = get_sblk (m_inode->i_dev)))
        crash ( "read inode from nonexistent dev !" );
    inode_blk_nr = m_inode->i_num / INODES_PER_BLK;
    inode_bit_nr = m_inode->i_num % INODES_PER_BLK;
    inode_blk_nr = 2 + sb->sb_imap_blocks + sb->sb_zmap_blocks + inode_blk_nr;
    if (!(bh = bread (m_inode->i_dev,inode_blk_nr)))
        crash ("read_dev_i bread failed !");
    memcpy (m_inode,((DEV_INODE*)bh->bh_buf) + inode_bit_nr,sizeof(DEV_INODE)) ;
    brelse (bh);        /* free */
    m_inode->i_valid = true;
}

LOCAL void write_dev_inode (MEM_INODE *m_inode) 
{
    u32 inode_blk_nr,inode_bit_nr;
    MEM_SUPER_BLOCK *sb ;
	BUFFER_HEAD *bh;

    if (!m_inode || !m_inode->i_dev || !m_inode->i_dirt) 
        return ;
    if ( !(sb = get_sblk (m_inode->i_dev)))
        crash ( "to write inode without dev !\n" );
    inode_blk_nr = m_inode->i_num / INODES_PER_BLK;
    inode_bit_nr = m_inode->i_num % INODES_PER_BLK;
    inode_blk_nr = 2 + sb->sb_imap_blocks + sb->sb_zmap_blocks + inode_blk_nr;
    if (!(bh = bread (m_inode->i_dev,inode_blk_nr)))
        crash ("read_dev_i bread failed !\n");
    memcpy (((DEV_INODE*)bh->bh_buf) + inode_bit_nr,m_inode,sizeof(DEV_INODE)) ;
    bh->bh_dirt = true;         
    brelse (bh);            /* sync */
    m_inode->i_dirt = false;
    m_inode->i_valid = true;
}

void sync_dev_inodes (void)
{
    u32 i;  
    for (i = 0 ; i < NR_INODE_BUF ; i++) {
        if (inode_table[i].i_cnt) continue;
        if (inode_table[i].i_dirt && inode_table[i].i_valid) {
			write_dev_inode (inode_table+i);			/* sync */
			memset (inode_table+i,0,sizeof(MEM_INODE));
		}
    }
}

MEM_INODE *get_empty_inode (void) 
{
    i32 i;
    while (true) {
        for (i = 0 ; i < NR_INODE_BUF ; i++) {
            if (inode_table[i].i_cnt) continue;
            if (inode_table[i].i_dirt && inode_table[i].i_valid)
                continue;
            memset (inode_table+i,0,sizeof(MEM_INODE));
            return (inode_table+i);
        }
        sync_dev_inodes ();
    }
    return (NULL);  
}

extern MEM_SUPER_BLOCK super_blk [NR_SBLKS];
extern void clear_iblks (MEM_INODE *m_inode) ;
extern i32 free_inode (MEM_INODE *inode) ;
extern MEM_INODE* new_inode (u16 dev_no) ;

void iput (MEM_INODE *m_puti) 
{
    if (!m_puti || !m_puti->i_cnt)	
        return ;
    if (!m_puti->i_dev) {			/* pipe */
        m_puti->i_cnt --;
        return;
    }
    if (!(-- m_puti->i_cnt)) {
        if (!m_puti->i_nlinks) {
            clear_iblks (m_puti);   
            free_inode (m_puti);   
            return ;                /* if free,no need to wb */
        }
		if (m_puti->i_dirt && m_puti->i_valid) 
			write_dev_inode (m_puti);	/* sync && free */
    }
}

LOCAL u8 getbit (u8 *addr,u32 bitno)
{
    u8 nr = bitno&7;

    addr += (bitno>>3); 
    return (((*addr) & (1<<nr)) >> nr);
}

MEM_INODE* iget (u16 dev_no,u32 inode_nr)
{
	MEM_INODE *m_inode;
    BUFFER_HEAD *tmp;
    u32 i,j;
    MEM_SUPER_BLOCK *sb ;
    /* The passed inode_nr must be validate (imap)
     * so,some chk before getting a inode is needed !
     */
    if (!(sb =  get_sblk (dev_no))) 
        crash ( "iget with bad dev or bad inr!" );
    if (!(tmp = sb->sb_imap_bh[inode_nr>>BLK_BITS_SFT]))
        crash ( "imap block %d not existent !",inode_nr>>BLK_BITS_SFT );
    if (!getbit (tmp->bh_buf,inode_nr & BLK_BITS_MSK)) {
        warn ( "iget a invalidate inode nr %d ",inode_nr );
        return (NULL);
    }
    for (i = 0; i < NR_INODE_BUF ;i ++) {	/* find in inode cache */
        m_inode = &inode_table[i];
		if (!m_inode->i_valid) 
            continue;
        if (m_inode->i_dev == dev_no && m_inode->i_num == inode_nr) {
            m_inode->i_cnt ++;
            if (m_inode->i_mounted) {       /* cd to mounted dir */
                for (j = 0 ; j < NR_SBLKS ; j++) 
                    if (super_blk [j].sb_i_mounted == m_inode) {
                        dev_no = super_blk [j].sb_dev;
                        inode_nr = ROOT_IND_NR;
                        iput (m_inode);
                        goto _READ_DEV;
                    }
            }
            return (m_inode);		
        }
    }
_READ_DEV:
    /* Now it's all safe to read a inode from dev 
     */
    if ((m_inode = get_empty_inode ())) {	/* if sad ,read from dev */
        memset (m_inode,0,sizeof(MEM_INODE));   
        m_inode->i_cnt   = 1;
        m_inode->i_dev   = dev_no;
        m_inode->i_num   = inode_nr;
        read_dev_inode (m_inode);  
        return (m_inode);       
	}
	return (NULL);		/* sad */
}

extern MEM_SUPER_BLOCK super_blk [NR_SBLKS];

void init_iscache (void) 
{
    memset (inode_table,0,sizeof(inode_table));
    memset (super_blk,0,NR_SBLKS * (sizeof(MEM_SUPER_BLOCK)));
}

extern MEM_INODE *new_inode (u16);
extern i32 free_inode (MEM_INODE *);
extern u32 new_block (u16);
extern void free_block (u16,u32);

LOCAL u32 _bmap (MEM_INODE *m_inode,u32 file_blk_no,u8 *fcrt) 
{
    BUFFER_HEAD *bh ;
    u32 blk_no = 0, *p = NULL;

    if (!m_inode || !m_inode->i_dev)
        crash ( "_bamp : bad dev" );
    if (!m_inode->i_valid)
        crash ("_bmap :bad inode (dev 0x%x,inr %d)!\n",m_inode->i_dev,m_inode->i_num); 
    if (file_blk_no >=  7 + NR_BLK_OBJS + NR_BLK_OBJS * NR_BLK_OBJS) 
        crash ( "_bmap : blk_no is too big !\n" );
    if (file_blk_no < 7) {			/* direct blks */
        if (*fcrt && !m_inode->i_zone[file_blk_no]) {
            if ((m_inode->i_zone[file_blk_no] = new_block (m_inode->i_dev))) {
				*fcrt = 0xFF;	
                m_inode->i_crt_time = CUR_TIME;
                m_inode->i_dirt = true;
            }
        }
        return (m_inode->i_zone[file_blk_no]);
    }
    file_blk_no -= 7 ;
    if (file_blk_no < NR_BLK_OBJS) {	/* 2nd direct blks */
        if (*fcrt && !m_inode->i_zone[7])  {
            if ((m_inode->i_zone[7] = new_block (m_inode->i_dev))) {
                m_inode->i_dirt = true;
                m_inode->i_crt_time = CUR_TIME;
            }
        }
        if ((!m_inode->i_zone[7])||!(bh = bread(m_inode->i_dev,m_inode->i_zone[7]))) 
            return (0);
        /* read  buffer for the reason that it may be not create block..
         * reading the old block buffer is also ok ..
         */
        p = (u32*)(bh->bh_buf);
        blk_no = p[file_blk_no];    
        if (*fcrt && !blk_no) {
            if ((blk_no = new_block (m_inode->i_dev))) {
                *fcrt = 0xFF;
                p[file_blk_no] = (u32)blk_no;
                bh->bh_dirt = true;
            }
        }
        brelse (bh);	/* sync */
        return (blk_no) ;
    }
    file_blk_no -= NR_BLK_OBJS ;
    if (*fcrt && !m_inode->i_zone[8]) {	/* 3nd diretc blks */
        if ((m_inode->i_zone[8] = new_block (m_inode->i_dev))) {
            m_inode->i_dirt = true;
            m_inode->i_crt_time = CUR_TIME;
        }
    }
    if ((!m_inode->i_zone[8])||!(bh = bread(m_inode->i_dev,m_inode->i_zone[8]))) 
        return (0);
    p = (u32*)(bh->bh_buf);
    blk_no = p[file_blk_no>>BLK_OBJ_SFT];		/* 2nd jump tab */
    if (*fcrt && !blk_no) {
        if ((blk_no = new_block (m_inode->i_dev))) {
            p[file_blk_no >> BLK_OBJ_SFT] = blk_no ;	/* blk nr */
            bh->bh_dirt = true;
        }
    }
    brelse (bh);
    if (!blk_no||!(bh = bread (m_inode->i_dev,blk_no))) 
        return (0);
    p = (u32*)(bh->bh_buf);
    blk_no = p[file_blk_no & BLK_OBJ_MSK];	/* blk_in nr */
    if (!blk_no && *fcrt) {
        if ((blk_no = new_block (m_inode->i_dev))) {
            *fcrt = 0xFF;
            p[file_blk_no & BLK_OBJ_MSK] = blk_no;
            bh->bh_dirt = true;
        }
    }
    brelse (bh);	/* sync */
    return (blk_no);
}

u32 bmap (MEM_INODE *m_inode,u32 file_blk_no) 
{
    u8 tmp = false;
    return (_bmap (m_inode,file_blk_no,&tmp));
}

u32 create_block (MEM_INODE *m_inode,u32 file_blk_no,u8 *created) 
{
	if (created) {
		*created = true;
		return (_bmap (m_inode,file_blk_no,created));
	}
	return (0);
}

/* this is ONLY called by truncate */
LOCAL void free_ind (u16 dev_no,u32 blk_no) 
{
    u32 *p = NULL,i = 0;

    if (!dev_no || !blk_no) 
        return ;
    BUFFER_HEAD *bh;

    if ((bh = bread (dev_no,blk_no))) {
        p = (u32*)(bh->bh_buf);
        for (i = 0 ; i < NR_BLK_OBJS ; i ++) 
            if (p[i]) 
                free_block (dev_no,p[i]);
        brelse (bh);
    }
    free_block (dev_no,blk_no);
}

LOCAL void free_dind (u16 dev_no,u32 blk_no) 
{
    if (!dev_no || !blk_no) 
        return ;
    BUFFER_HEAD *bh;
    u32 *p,i = 0 ;

    if ((bh = bread (dev_no,blk_no))) {
        p = (u32*)(bh->bh_buf);
        for (i = 0 ; i < NR_BLK_OBJS ; i ++) 
            if (p[i]) 
                free_ind (dev_no,p[i]);
        brelse (bh);
    }
    free_block (dev_no,blk_no);
}

void clear_iblks (MEM_INODE *m_inode) 
{
    u8 i = 0;

    if (!m_inode ||!m_inode->i_dev) 
        return ;
    if (!S_ISREG (m_inode->i_mode) || S_ISDIR(m_inode->i_mode)) 
        return;
    for (i = 0 ;  i < 7 ; i ++) { 
        if (m_inode->i_zone[i]) {
            free_block (m_inode->i_dev,m_inode->i_zone[i]);
            m_inode->i_zone[i] = 0;
		}
    }
    free_ind (m_inode->i_dev,m_inode->i_zone[7]);
    free_dind(m_inode->i_dev,m_inode->i_zone[8]);
    m_inode->i_zone[7] = m_inode->i_zone[8] = 0;
    m_inode->i_size = 0;
    m_inode->i_mtime = m_inode->i_acc_time = CUR_TIME;
}

void free_file_blk (MEM_INODE *m_inode,u32 file_blk_no) 
{
    if (!m_inode) 
        return;
    BUFFER_HEAD *bh ;
    u32 blk_no = 0;
    u32 *p = NULL;

    if (!m_inode->i_dev)
        crash ( "free file blk %d on non-existent dev.\n",file_blk_no );
    if (!m_inode->i_cnt)
        crash ( "free from invalidate inode\n" ); 
    if (file_blk_no >=  7 + NR_BLK_OBJS + NR_BLK_OBJS * NR_BLK_OBJS) 
        crash ( "free_file_blk : blk_no is too big !\n" );
    if (file_blk_no < 7) {
        free_block (m_inode->i_dev,m_inode->i_zone[file_blk_no]);
        m_inode->i_zone[file_blk_no] = 0;
        m_inode->i_dirt = true;
        return ;
    }
    file_blk_no -= 7 ;
    if (file_blk_no < NR_BLK_OBJS) {
        if (!m_inode->i_zone[7])    /* if failed ! */
            return ;
        if (!(bh = bread (m_inode->i_dev,m_inode->i_zone[7]))) 
            return ;
        p = (u32*)(bh->bh_buf);
        blk_no = p[file_blk_no];    /* this must be here,logical short cut ! */
        free_block (m_inode->i_dev,blk_no);
        p[file_blk_no] = 0;
        bh->bh_dirt = true;
        brelse (bh);
        return ;
    }
    file_blk_no -= NR_BLK_OBJS ;
    if (!m_inode->i_zone[8])
        return ;
    if (!(bh = bread (m_inode->i_dev,m_inode->i_zone[8]))) 
        return ;
    p = (u32*)(bh->bh_buf);
    blk_no = p[file_blk_no>>BLK_OBJ_SFT];		/* avoid loagical shut cut */ 
    brelse (bh);
    if (!blk_no) 
        return ;
    if (!(bh = bread (m_inode->i_dev,blk_no))) 
        return ;
    p = (u32*)(bh->bh_buf);
    blk_no = p[file_blk_no & BLK_OBJ_MSK];
    free_block (m_inode->i_dev,blk_no);
    p[file_blk_no & BLK_OBJ_MSK] = 0;
    bh->bh_dirt = true;
    brelse (bh);
}
