#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <ecasey/fs.h>

LOCAL INLINE u8 setb (u8 *addr,u32 bitno)
{
    u8 ret,nr = bitno&7;

    addr += (bitno>>3); 
    ret = ((*addr) & (1<<nr)) >> nr;
    *addr |= (1<<nr);
    return (ret);
}

LOCAL INLINE u8 clrb (u8 *addr,u32 bitno) 
{
    u8 ret,nr = bitno&7;

    addr += (bitno>>3); 
    ret = ((*addr) & (1<<nr)) >> nr;
    *addr &= ~(1<<nr);
    return (ret);
}
 
LOCAL INLINE u32 find_first_zero (u8 *p,u32 bitno)
{
    u32 i = bitno>>3 ,j = bitno&7 ;
    /* this is not so good but can just work out */
    for (; i < BLK_SIZE ;i ++ ) {
        if (0xFF == p[i]) continue;            
        for (; j < 8 ; j ++) 
            if (!(p[i] & (1<<j)))
                return ((i<<3) + j);
        j = 0;
    }
    return (-1);                    
}

extern MEM_SUPER_BLOCK *get_sblk (u16);
extern int crash (const char *fmt,...);
extern BUFFER_HEAD *get_blk (u16,u32);
extern void brelse (BUFFER_HEAD*);

void free_block (u16 dev_no,u32 blk_nr) 
{
    MEM_SUPER_BLOCK *sb = NULL;
    BUFFER_HEAD *tmp = NULL;

    if (!dev_no || !(sb = get_sblk (dev_no))) 
        crash ( "free block on nonexistent dev !\n" );
    if (blk_nr < sb->sb_first_datazone || blk_nr >= sb->sb_nzones) 
        crash ("free non-zoned zone !\n") ;
    if ((tmp = get_blk (dev_no,blk_nr))) {
        if (tmp->bh_cnt != 1) 
            crash ("free block %d,count = %d\n",blk_nr,tmp->bh_cnt) ;
        tmp->bh_dirt = tmp->bh_valid = false;   /* not sync  */
        brelse (tmp);
    }
    blk_nr -= sb->sb_first_datazone;    /* data zone nr */
    if (!clrb (sb->sb_zmap_bh[blk_nr>>BLK_BITS_SFT]->bh_buf,blk_nr&BLK_BITS_MSK)) 
        crash ( "bmap (blk_nr : %d) already free !\n",blk_nr );
    sb->sb_zmap_bh[blk_nr>>BLK_BITS_SFT]->bh_dirt = true;   /* sync */ 
}

u32 new_block (u16 dev_no) 
{
    MEM_SUPER_BLOCK *sb;
    BUFFER_HEAD *tmp ;
    u32 i,j,k,l;

    if (!dev_no || !(sb = get_sblk (dev_no))) 
        crash ("new block from nonexistent dev!\n");
    j = sb->sb_pos >> BLK_BITS_SFT;
    k = sb->sb_pos  & BLK_BITS_MSK;
    for (i = 0 ; i < sb->sb_zmap_blocks ; i ++) { 
        if ((tmp = sb->sb_zmap_bh[j])) {
            l = find_first_zero (tmp->bh_buf,k);
            if ((l != (-1)) && (((j << BLK_BITS_SFT) + l + sb->sb_first_datazone) < sb->sb_nzones))
                break;
        }
        k = 0;
        j = (j+1) % sb->sb_zmap_blocks;
    }
    if (i >= sb->sb_zmap_blocks || !tmp || l == (-1)) /* fall through */
        return (0);
    if (setb (tmp->bh_buf,l)) 
        crash ("new block: bit %d already busy!\n",l);
    tmp->bh_dirt = true;            /* modified */
    sb->sb_pos = (j << BLK_BITS_SFT) + l;
    l += (j << BLK_BITS_SFT) + sb->sb_first_datazone;
    if (l >= sb->sb_nzones) 
        return (0);
    if (!(tmp = get_blk (dev_no,l))) 
        crash  ("new block : get blk failed !\n");
    memset (tmp->bh_buf,0,BLK_SIZE);
    tmp->bh_dirt = tmp->bh_valid = true;   /* always be write*/
    brelse (tmp);           
    return (l);
}
extern MEM_INODE *get_empty_inode (void);

MEM_INODE* new_inode (u16 dev_no) 
{
    MEM_SUPER_BLOCK *sb ;
    BUFFER_HEAD *imap;
    MEM_INODE *m_inode;
    u32 i,j = BLK_SIZE << 3;

    if (!(sb = get_sblk (dev_no))) 
        crash ( "new inode from nonexistent dev!\n" ) ;
    for (i = 0 ; i < sb->sb_imap_blocks ; i ++)
        if ((imap = sb->sb_imap_bh[i])) 
            if ((j = find_first_zero (imap->bh_buf,0)) != (-1)) 
                break;
    if (i >= sb->sb_imap_blocks || !imap || j == (-1)) 
        return (NULL);
    if (setb (imap->bh_buf,j)) 
        crash ( "new inode : bit %d already set !\n",(i << BLK_BITS_SFT) + j );
    imap->bh_dirt = true;           /* sync */
    if (!(m_inode = get_empty_inode ()))
        crash ( "get empty inode failed !\n" );
    m_inode->i_cnt      = 1;
    m_inode->i_dirt     = true;     /* always write */
    m_inode->i_valid    = true;
    m_inode->i_nlinks   = 1;
    m_inode->i_dev      = dev_no;
    m_inode->i_locked   = false;
    m_inode->i_magic    = I_MAGIC;
    m_inode->i_uid      = UID;        
    m_inode->i_gid      = GID;
    m_inode->i_num      = (i<<BLK_BITS_SFT) + j ;/* inode nr */
    m_inode->i_mtime    = m_inode->i_acc_time = m_inode->i_crt_time = CUR_TIME;
    return (m_inode);
}

i32 free_inode (MEM_INODE *inode) 
{
    MEM_SUPER_BLOCK *sb ;
    BUFFER_HEAD *tmp ;

    if (!inode) 
        return (-1);
    if (!(sb =  get_sblk (inode->i_dev))) 
        crash ( "free inode from nonexistent dev !\n" ) ;
    if (!inode->i_dev) {
        memset (inode,0,sizeof(MEM_INODE));     /* free inode_table */
        return (-1);
    }
    if (inode->i_cnt || inode->i_nlinks)
        crash ( "free busy inode");
    if (!(sb = get_sblk (inode->i_dev)))
        crash ( "free nonexistent dev inode !\n" );
    if (!(tmp = sb->sb_imap_bh[inode->i_num>>BLK_BITS_SFT]))
        crash ( "imap block %d not existent !\n",inode->i_num>>BLK_BITS_SFT );
    if (!clrb (tmp->bh_buf,inode->i_num&BLK_BITS_MSK))
        crash ( "free_inode : bit %d already free !\n",inode->i_num&BLK_BITS_MSK );
    tmp->bh_dirt = true;                        /* sync */
    memset (inode,0,sizeof(MEM_INODE));         /* free inode table*/
    return (0);
}
