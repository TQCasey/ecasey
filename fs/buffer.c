/* buffer cache for caseyFS v2
 * 2013 - 7 - 31 by casey 
 */
#include <sys/types.h>
#include <ecasey/fs.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <2440/ports.h>

#define BUFFER_SIZE     (FS_BUFFER_SIZE)
#define BUF_START       (USR_FS_BUFFER)
#define BUF_END         (BUF_START + BUFFER_SIZE)       
#define NR_HASH         254
#define OUT_BLKS        20

i32 NR_BUFFERS = 0;
LOCAL BUFFER_HEAD *free_list  = NULL;           
LOCAL BUFFER_HEAD *hash[NR_HASH] = {0};

#define get_irow(dev_no,blk_no)         (((dev_no)^(blk_no))%NR_HASH)
#define hash_row(dev_no,blk_no)         (hash[get_irow (dev_no,blk_no)])

extern int crash (const char *fmt,...);
extern u32 sync_blks (u32 blks_cnt) ;

LOCAL BUFFER_HEAD* add_hash (BUFFER_HEAD *p) 
{  
    u16 dev_no = p->bh_dev;
    u32 blk_no = p->bh_blk_nr;
	i32 i = get_irow (dev_no,blk_no);

	if (!p || !p->bh_dev)
        crash ( "bad buffer argument !\n" );
	if (!hash[i]) {          /* first one */
		p->hash_next = p->hash_prev = p;
		hash[i] = p;
	} else {                                    /* appened */
        hash[i]->hash_prev->hash_next = p;
        p->hash_prev = hash[i]->hash_prev;
        hash[i]->hash_prev = p;
        p->hash_next = hash[i];
    }
	p->bh_hashed = true;
    return (p);
}

BUFFER_HEAD *get_buffer (u16 dev_no,u32 blk_no)
{
	BUFFER_HEAD *ret = NULL;

    while (true) {
        if (free_list) {
            ret = free_list->free_next;
            if (ret == free_list)               /* last one */
                free_list = NULL ;
            else {                              
                free_list->free_prev->free_next = free_list->free_next;
                free_list->free_next->free_prev = free_list->free_prev;
                ret = free_list;
                free_list = free_list->free_next;
            }
            ret->free_next = ret->free_prev = NULL;
            ret->bh_dev = dev_no;
            ret->bh_blk_nr = blk_no;
            return (add_hash (ret)) ;
        } else
            sync_blks (OUT_BLKS);   /* sync some blks */
    }
	return (ret);
}

BUFFER_HEAD *find_buffer (u16 dev_no,u32 blk_no)
{
	BUFFER_HEAD *q,*p;

	i32 i = get_irow(dev_no,blk_no);
	if (!hash[i]) 
        return (NULL);
	q = hash[i]->hash_prev ;
	p = hash[i];

	while ( true ) {
		if (dev_no == p->bh_dev && blk_no == p->bh_blk_nr)
            return (p);
		if (dev_no == q->bh_dev && blk_no == q->bh_blk_nr)
            return (q);
		if (p->hash_next == q) break;	
		if (p == q) break;				
		p = p->hash_next;
		q = q->hash_prev;				
	}
	return (NULL);
}

void free_buffer  (BUFFER_HEAD *p) 
{
    BUFFER_HEAD *q;
	i32 i = get_irow (p->bh_dev,p->bh_blk_nr);
	if (!p || !p->bh_dev || !(q = hash[i])) 
        return ;
    if (p->bh_cnt) 
        crash ( "free buffer dev 0x%x,blk_no %d with count = %d !\n",
                p->bh_dev,p->bh_blk_nr,p->bh_cnt );
	if (p->hash_next == p)	/* last one */
		hash[i] = NULL;	    
	else {
		p->hash_prev->hash_next = p->hash_next;
		p->hash_next->hash_prev = p->hash_prev;
		if (hash [i] == p) 	/* head */
			hash [i] = p->hash_next;
	}
	if (!free_list){    /* first to add */ 
		p->free_next = p->free_prev = p;
		free_list = p;
	} else {
	    free_list->free_prev->free_next = p;
	    p->free_prev = free_list->free_prev;
	    p->free_next = free_list;
	    free_list->free_prev = p;
    }
    p->bh_cnt       = 0;
    p->bh_dirt      = false;    
    p->bh_valid     = false;
    p->bh_locked    = false;
    p->bh_blk_nr    = 0;        
    p->bh_dev       = 0;
    p->hash_next    = p->hash_prev  = NULL;
	p->bh_hashed = false;               /* clean up done ! */	
}

i32 init_buffer (void) 
{
    BUFFER_HEAD *bh = (BUFFER_HEAD*)BUF_START;      
    u8 *buf = (u8*)(BUF_END - BLK_SIZE);        
    i32 i = 0;
    //NR_BUFFERS = 0;

    while ( (u8*)(bh+1) <= (buf) ) {
        bh->bh_dev      = 0;
        bh->bh_dirt     = 0;
        bh->bh_locked   = false;
        bh->bh_valid    = false;
        bh->bh_cnt      = 0;
        bh->bh_buf      = buf;   
		bh->bh_hashed	= false;
        bh->bh_blk_nr   = 0;                        
        bh->free_prev   = bh-1;                     
        bh->free_next   = bh+1;                     
        bh ++;                                      
        buf -= BLK_SIZE;                            
        NR_BUFFERS ++;                              
    }
    free_list = (BUFFER_HEAD*)BUF_START;            
    free_list->free_prev = --bh;                
    bh->free_next = free_list;                  
    while (i < NR_HASH) hash[i++] = NULL;
    return (NR_BUFFERS);
}

void brelse (BUFFER_HEAD *bh_free)
{
    if (!bh_free) return ;
    if (!(bh_free->bh_cnt --)) 
        crash ( "brelse free buffer !\n" );
    if (!bh_free->bh_cnt) 
        if (!bh_free->bh_dirt || !bh_free->bh_valid)
            free_buffer (bh_free);      /* no wb ,a new fresh block */
}

/* get a blk buffer  */
BUFFER_HEAD *get_blk (u16 dev_no,u32 blk_no) 
{
    BUFFER_HEAD *tmp = NULL;

    if (!(tmp = find_buffer (dev_no,blk_no))) {     /* cached ? */ 
        if  ((tmp = get_buffer (dev_no,blk_no))) {  
            tmp->bh_locked = false;                 /* no */
            tmp->bh_cnt ++;         
            tmp->bh_dev = dev_no;
            tmp->bh_valid = false; 
            tmp->bh_dirt = false;               
            tmp->bh_blk_nr = blk_no;
            return (tmp);
        }
        else    
            return (NULL);  /* BAD BAD BAD */
    }
    tmp->bh_cnt ++; 
    return (tmp);
}

extern i32  ll_rw_blk (u8,u16,u32,u32,void*);

BUFFER_HEAD *bread (u16 dev_no,u32 blk_no) 
{
    BUFFER_HEAD *tmp ;
    if (!(tmp = get_blk (dev_no,blk_no)))
        crash ( "bread : no more free buffer !\n" );
    if (tmp->bh_valid) 
        return (tmp);       /* happy if valid */
    if (!ll_rw_blk (READ,dev_no,blk_no,1,(void*)(tmp->bh_buf)))
        return (NULL);
    tmp->bh_valid = true;   /* READ :not dirt but valid */
    return (tmp);
} 

/* stop if a negative number met*/
BUFFER_HEAD* breada (u16 dev_no,u32 blk_no ,...) 
{
    va_list va_p ;
    BUFFER_HEAD *bh,*tmp;
    va_start (va_p,blk_no);

    if (!(bh = bread (dev_no,blk_no))) 
        crash ( "breada : bread failed !\n" );
    while ((blk_no = va_arg(va_p,u32)) >= 0) 
        if ((tmp = bread (dev_no,blk_no)) && tmp->bh_valid)
                tmp->bh_cnt --;
    return (bh);
}

/* sync all dev buffers */
void sync_dev_blks (u16 dev_no)
{
    u32 i;
    BUFFER_HEAD *bh = (BUFFER_HEAD*)BUF_START;

    for (i = 0;i < NR_BUFFERS ; i++) {
        if (bh[i].bh_dirt && bh[i].bh_valid && (bh[i].bh_dev == dev_no)) {
            ll_rw_blk (WRITE,bh[i].bh_dev,bh[i].bh_blk_nr,1,bh[i].bh_buf);
            bh[i].bh_dirt = false;          /* sync to dev */
            if (!bh[i].bh_cnt) 
                free_buffer (&bh[i]) ;  /* free buffer */
        }
    }
}

LOCAL u32 pos = 0;      /* sync postion ,ROBIN ROUND */

/* genernal syc_block call */
u32 sync_blks (u32 blks_cnt) 
{
    u32 cnt = 0,i = 0;
    BUFFER_HEAD *bh = (BUFFER_HEAD*)BUF_START;

	if (blks_cnt > NR_BUFFERS)
		blks_cnt = NR_BUFFERS - 1;
    for (i = 0; i < NR_BUFFERS; i ++) {
		pos = (pos + 1) % NR_BUFFERS;
		if (!bh[pos].bh_hashed)     /* not cached */ 
            continue;
        if (bh[pos].bh_dirt && bh[pos].bh_valid) {
            ll_rw_blk (WRITE,bh[pos].bh_dev,bh[pos].bh_blk_nr,1,bh[pos].bh_buf);
            bh[pos].bh_dirt = false;            /* clean  */
        }
        if (!bh[pos].bh_cnt) { 
            free_buffer (&bh[pos]) ;    /* free buffer */
             if ((cnt ++ >= blks_cnt)) 
				 break;
        }
    }
    return (cnt);
}
