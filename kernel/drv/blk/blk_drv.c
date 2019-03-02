#include <2440/2440addr.h>
#include <2440/drvs.h>
#include <ecasey/kernel.h>
#include <sys/types.h>
#include <ecasey/mail.h>
#include <ecasey/fs.h>
#include <string.h>

#define BAD_BLK_FLAG        (0x33)

extern i32 nand_reset (void);
i32 nand_reset (void)
{
    NF_CE_ON ();                        /* chip on */
    NF_CLR_RB ();                       /* clear R/B */
    NF_CMD (NF_RESET);                  /* RESET */
    NF_WAIT_RB ();                      /* wait till not busy */
    NF_CE_OFF ();                       /* chip off */
    return (0);
}

u32 nand_read_page (void *to,u32 start_addr)
{
    if (!to) return (0);
    u8 *p = (u8*)to;
    i32 i ;
    NF_CE_ON ();                        /* chip on */
    NF_CLR_RB ();                       /* clear state */
    NF_CMD (NF_READ1);                  /* NAND READ */
    NF_ADDR (0x00);
    NF_ADDR (0x00);                     /* block nr == 0 */
    NF_ADDR((start_addr>>0)&0xFF);	    /* page nr low8 */
	NF_ADDR((start_addr>>8)&0xFF);	    /* mid8 */
	NF_ADDR((start_addr>>16)&0xFF);     /* high8 */
    NF_CMD (NF_READ2);                  /* 0x30 */
    NF_WAIT_RB ();                      /* wait till not busy */
    for (i = 0; i < BYTES_PER_PAGE ; i ++) 
        p [i] = nand_read_byte ();      /* nand flash read byte */
    NF_CE_OFF ();                       /* chip off */
    return (1);
}

/* read imgs from nand flash to SDRAM */
u32 copy_nand_to_sdram (u32 sdram_addr,u32 nand_addr,u32 img_size)
{
    i32 pages = ((img_size & 0x7FF) != 0) + (img_size >> 11),i = 0;
    for (i = 0 ; i < pages ; i ++,sdram_addr += 2048 )
        nand_read_page ((void*)sdram_addr,(nand_addr >> 11) + i);
    return (0);
}

i8 nand_get_id (void)
{
	i8 pMID,pDID,tmp;

	NF_CE_ON();					        /* chip ON */    
	NF_CLR_RB();				        /* clear RnB */
	NF_CMD(NF_READ_ID);			        /* read id 0x90 */
	NF_ADDR(0x00);				        /* write 0x00 */
	pMID = nand_read_byte();		    /* MID */
	pDID = nand_read_byte();		    /* ID */
	tmp	 = nand_read_byte();			
	tmp	 = nand_read_byte();	
	tmp  = nand_read_byte();
	NF_CE_OFF();				        /* chip disable */
	return (pDID);
}

/* return 1 for OK, 0 for FAILED  */
u8 nandx_write_byte (u32 page_nr,u32 addr,u8 dat)
{
	u8 stat;

	NF_CE_ON ();                        /* chip on */
	NF_CLR_RB();                        /* clr RnB */
	NF_CMD(NF_PAGE_WRITE1);             /* write 1st */
	NF_ADDR(0x00);                      /* blk nr == 0 */
	NF_ADDR(0x00);                      /* blk nr == 0 */                      
	NF_ADDR((page_nr)&0xff);            /* page nr low8 */
	NF_ADDR((page_nr>>8)&0xff);         /* page nr mid8 */
	NF_ADDR((page_nr>>16)&0xff);        /* page nr high8 */
	NF_CMD(NF_RANDOM_WRITE);            /* random write */
	NF_ADDR((i8)(addr&0xff));         /* page in offset */ 
	NF_ADDR((i8)((addr>>8)&0x0f));
	nand_write_byte(dat);    	        /* write data */
	NF_CMD(NF_PAGE_WRITE2);             /* write 2nd */
	NF_CMD(NF_READ_STATUS);             /* read status */ 
	while (!((stat =  nand_read_byte ()) & 0x40));	
	NF_CE_OFF ();				        /* chip OFF */
	return (0 == (stat&0x01));
}
u8 nandx_read_byte (u32 page_nr,u32 addr)
{
	u8 stat;
	NF_CE_ON ();                        /* chip on */
	NF_CLR_RB();                        /* clr RnB */
	NF_CMD(NF_READ1);                   /* read 1st */
	NF_ADDR(0x00);                      /* blk nr == 0 */
	NF_ADDR(0x00);                      /* blk nr == 0 */                      
	NF_ADDR((page_nr)&0xff);            /* page nr low8 */
	NF_ADDR((page_nr>>8)&0xff);         /* page nr mid8 */
	NF_ADDR((page_nr>>16)&0xff);        /* page nr high8 */
    NF_CMD(NF_READ2);                   /* PAGE read 2 */
    NF_WAIT_RB();                       /* till RnB not busy */
	NF_CMD(NF_RANDOM_READ1);            /* random read1 */
	NF_ADDR((i8)(addr&0xff));           /* page in offset */ 
	NF_ADDR((i8)((addr>>8)&0x0f));
    NF_CMD(NF_RANDOM_READ2);            /* Random read2 */
    stat = nand_read_byte ();
	NF_CE_OFF ();				        /* chip OFF */
	return (stat);                      /* read byte */
}

/* 1 for bad block ,else good block ! */
u8 nandx_is_badblk (u32 block)
{
	/* blk page_nr == 0,2048 +  6 bytes */
    return (0xFF != nandx_read_byte ((block<<6),2048 + 6 ));
}

/* to write 0x33 to the 6th byte in spareZONE in first block 
 * return 1 for OK ,0 for FAILED 
 */
u8 nandx_mark_badblk (u32 block)
{
    return (nandx_write_byte (block<<6, 2048+6, BAD_BLK_FLAG));
}

/* 0 for ok,-1 for bad opt */
i8 nandx_read_page (u8 *to,u32 size,u32  page_nr)
{
    if (!to || !size) 
        return (-1);             
	u32 i,j,mecc0,secc;
	
    NF_RSTECC ();				        /* reset ECC */
    NF_MECC_Unlock ();			        /* unlock main blk ecc */
    NF_CE_ON ();				        /* Chip ON */
    NF_CLR_RB();				        /* busy */
    NF_CMD(NF_READ1);			        /* NAND READ */
    NF_ADDR(0x00);				        /* blk */
    NF_ADDR(0x00);				        /* blk nr == 0 (12bits)*/
    NF_ADDR((page_nr) & 0xff);	        /* page nr low8 */
    NF_ADDR((page_nr>>8)&0xff);	        /* mid8 */
    NF_ADDR((page_nr>>16)&0xff);        /* high8 */
    NF_CMD(NF_READ2);			        /* 2nd read T(0x030)*/
    NF_WAIT_RB();				        /* wait till RnB not busy */
	if (size >= BYTES_PER_PAGE)
		size = BYTES_PER_PAGE;	        /* truncate */
    for (i = 0,j = 0; i < BYTES_PER_PAGE; i++) { 
            if (j  < size)              /* always fully read */ 
			    to[j++] =  nand_read_byte ();
    }
	NF_MECC_Lock();				        /* lock main blk ecc */
	NF_SECC_Unlock();			        /* unlock spare blk ecc */
    mecc0 = NF_RDMECC0();               /* read main ecc */ 
	rNFMECCD0 = ((mecc0&0xff00)<<8)|(mecc0&0xff);
	rNFMECCD1 = ((mecc0&0xff000000)>>8)|((mecc0&0xff0000)>>16);
	NF_SECC_Lock();       		        /* lock SECC */        
	secc = NF_RDSECC ();                /* read secc */
	rNFSECCD=((secc&0xff00)<<8)|(secc&0xff);
	NF_CE_OFF();				        /* chip off */
    return ((rNFESTAT0&0xf));
}

/* to write a page ,0 for OK, 1 for bad blk ,2 for makr err*/
i8 nandx_write_page (u32 page_nr,u8 *src,u32 size)
{
	u32 i,j, mecc0,secc;
	i8 stat;
    i32 dly = 1000;

	if (nandx_is_badblk (page_nr>>6) || !src || !size)
		return (1);					    /* bad block */
	NF_RSTECC();                   	    /* reset ECC */
	NF_MECC_Unlock();          		    /* unlock main ECC */
	NF_CE_ON ();            		    /* chip ON */
	NF_CLR_RB();        			    /* clr rnb */	
	NF_CMD(NF_PAGE_WRITE1);             /* write 1st */
	NF_ADDR(0x00);                 	    /* blk nr == 0 */
	NF_ADDR(0x00);                  
	NF_ADDR((page_nr) & 0xff); 		    /* page nr low8 */
	NF_ADDR((page_nr>>8)&0xff);   	    /* mid8 */
	NF_ADDR((page_nr>>16)&0xff);  	    /* high8 */
	if (size >= BYTES_PER_PAGE)
		size = BYTES_PER_PAGE;		    /* trancate */
	for (i = 0,j = 0; i < BYTES_PER_PAGE; i++)
		if (j < size)  
	    	nand_write_byte(src[j++]);	/* write byte */
	NF_MECC_Lock();                     /* lock main ECC */
	mecc0 = NF_RDSECC ();               /* read main ECC */
	NF_SECC_Unlock();                   /* unlock secc */
	for( i = 0; i < 4; i++,mecc0 >>= 8 ) 
		nand_write_byte (mecc0&0xFF);	/* mecc write to spare first 4 bytes */
	NF_SECC_Lock();                     /* lock secc */
	secc = NF_RDSECC();                 /* read secc */
	nand_write_byte (secc&0xFF);		/* secc 4th,5th byte */
	nand_write_byte ((secc>>8)&0xFF);
	NF_CMD(NF_PAGE_WRITE2);             /* write 2nd */
    while (dly --);                     /* wait tWB */
	NF_CMD(NF_READ_STATUS);      
	while (!((stat =  nand_read_byte ()) & 0x40));
	NF_CE_OFF ();					    /* CHIP OFF */
	if (stat & 0x1){
		if (nandx_mark_badblk (page_nr>>6))
		    return (2);					/* mark ok */       
        return (3);                     /* mark failed */
	}
	return (0);                  	    /* WRITE OK */
}
/* To wipe out the whole blk data
 * 0 for OK, 1 for bad blk ,2 for mark err
 */
i8 nandx_erase_blk (u32 blk_nr)
{
    i8 stat;
    i32 dly = 1000;

    if (nandx_is_badblk (blk_nr))
        return (1);                     /* erase bad blok failed */
    NF_CE_ON ();
    NF_CLR_RB ();
    NF_CMD (NF_BLK_ERASE1);             /* erase cmd 1 */
	NF_ADDR((blk_nr<<6) & 0xff); 		/* A18 - A19 */
	NF_ADDR((blk_nr>>2) & 0xff);   	    /* A20 - A27 */
	NF_ADDR((blk_nr>>10) & 0xff);  	    /* A28 */
    NF_CMD (NF_BLK_ERASE2);             /* erase cmd 2 */
    while (dly --);                     /* wait tWB (100ns) */
    NF_CMD (NF_READ_STATUS);            /* read status command */
	while (!((stat =  nand_read_byte ()) & 0x40));
    NF_CE_OFF ();                       /* chip off */
	if (stat & 0x1){
		if (nandx_mark_badblk (blk_nr))
		    return (2);					/* mark ok */       
        return (3);                     /* mark failed */
	}
	return (0);                  	    /* WRITE OK */
}

typedef struct _tl_cache_st {           /* translation layer cache structure */
    i16     blkno;                      /* physical blk nr */
    i8      *buf;                       /* buffer */
    i8      dirt;                       /* is it dirt ? */
    u32     atime;                      /* latest access time */
}tl_cache_st;

LOCAL tl_cache_st *tl_cache = (void*)0;

typedef struct _map_tab_st {
    i16     valid;
    i16     dirt; 
    i16     good_blk[BLKS_PER_NANDF];
    i16     good_cnt;
    i16     bad_blk[MAX_BAD_BLKS];
    i16     bad_cnt;
}map_tab_st;

#define MAP_TAB_BLK_NO      4
LOCAL map_tab_st *map_tab = (void*)0;

INLINE i16 l2pblkno (i16 lblkno)
{
    if (lblkno < 0 || lblkno >= map_tab->good_cnt)
        return (-1);
    return (map_tab->good_blk[lblkno]);
}
extern i8 nandx_read_page (u8 *to,u32 size,u32  page_nr);

i8 get_map_tab (void)
{
    i8 state,*p ;
    i32 i = 0,j,k ;

    map_tab = (map_tab_st*)(MAP_TAB);
    p = (i8*)(MAP_TAB);
    /* i : page nr 
     * j : read bytes per cycle 
     * k : remain size
     */
    k = sizeof(map_tab_st);
    while (k > 0) {
        j = BYTES_PER_PAGE;
        if (j > k) j = k;
        k -= j;
        if ((state  = nandx_read_page ((u8*)(p + (i << 11)),j,(MAP_TAB_BLK_NO<<6)+i)))
            return (state);
        i ++;
    }
    /* clean */
    if (map_tab->valid == (-1)) {
        printk ( "$ > fresh map_tab,rebuilding.... !\n" );
        for (i = 48,j = k = 0; i < BLKS_PER_NANDF; i++) {
            if (nandx_is_badblk (i)) {
                map_tab->bad_blk[j ++] = i;
                continue;
            }
            map_tab->good_blk[k ++] = i;
        }
        map_tab->good_cnt = k;
        map_tab->bad_cnt = j;
        map_tab->dirt = map_tab->valid = 1;
    }
    return (state);
}
extern i8 nandx_write_page (u32 page_nr,u8 *src,u32 size);

/* release map_tab */
i8 put_map_tab (void)
{
    i8 state = 0;
    i32 i = 0,j,k ;
    
    map_tab = (map_tab_st*)(MAP_TAB);
    i8 *p = (i8*)(MAP_TAB);
    
    if (1 != map_tab->dirt)     /* clean */
        return (state);
    /* i : page nr 
     * j : read bytes per cycle 
     * k : remain size
     */
    k = sizeof(map_tab_st);
    while (k > 0) {
        j = BYTES_PER_PAGE;
        if (j > k) j = k;
        k -= j;
        if ((state  = nandx_write_page ((MAP_TAB_BLK_NO<<6)+i,(u8*)(p + (i << 11)),j)))
            return (state);
        i ++;
    }
    map_tab->dirt = -1;         /* be clean */
    return (state);
}

i8 dump_map_tab (void)
{
    i32 i = 0;
    i8 state ;

    if ((state = get_map_tab ())) 
        return (state);
    for (i  = 0 ; i < map_tab->bad_cnt ;i++) 
        printk ( "# > Bad block nr [ %08d ] \n",map_tab->bad_blk[i]);
    return (state);
}

/* size default 128K ,ONLY called by get_blk () */ 
i8 read_blk (void *buf,i32 blk_nr)
{
    i32 i = 0;
    i8 *p = (i8*)buf,state;

    for (i = 0; i < PAGES_PER_BLK ; i++) {
        if ((state = nandx_read_page ((u8*)p,BYTES_PER_PAGE,(blk_nr << 6) + i)))
            return (state);
        p += BYTES_PER_PAGE;
    }
    return (state);
}

/* size default 128K ,ONLY called by put_blk () */ 
LOCAL i8 write_blk (i32 blk_nr,void *src)
{
    i32 i = 0;
    i8 *p = (i8*)src,state;

    if ((state = nandx_erase_blk (blk_nr)))               /* erase first */
        return (state);
    for (i = 0; i < PAGES_PER_BLK; i++) {
        if ((state = nandx_write_page ((blk_nr<<6)+i,(u8*)p,BYTES_PER_PAGE)))
            return (state);
        p += BYTES_PER_PAGE;
    }
    return (0);
}

i32 blk_drv_init (void)
{
    i32 i,tmp;
    /* nand flash info */
    printk ( "$ > Nand flash : PID : 0x%X \n",nand_get_id () );
    /* configure out bad blocks */
    tl_cache = (tl_cache_st*)(TL_CACHE_HEAD);
    /* dump the map_tabs */
    dump_map_tab ();

    for (i = 0,tmp = TL_BUFFER; i < CACHE_ST_NR ; i ++) {
        tl_cache[i].blkno = -1;         /* no refer */
        tl_cache[i].buf = (i8*)tmp;     /* buffer */
        tl_cache[i].dirt = 0;           /* clean */
        tmp += BLK_BUF_SIZE;
    }
    return (0);
}

extern i32 panic (i8 *fmt,...);

#define NAND_BLK_SFT    (5)             /* NAND BLOCK / 32 FILE BLOCKS*/
#define NAND_BLK_MASK   (31)

LOCAL i16 sync_tl_cache (void)
{
    i16 min = 0,i = 0;

    for (i = 1 ; i < 8 ; i++) 
        if (tl_cache [min].atime > tl_cache [i].atime)
            min = i;
    if (write_blk (tl_cache[min].blkno,(void*)tl_cache [min].buf))
        return  (-1);
    return (min);
}

i32 sys_rw_nand (mail_st *m)
{
    u32 rw,fblkno,blkno,*p = (u32*)m->src,i;
    void *buf,*src,*to;

    rw      = p[0];
    fblkno  = p[1];
    buf     = (void*)p[2];

    blkno = fblkno >> NAND_BLK_SFT;
    if ((blkno = l2pblkno (blkno)) < 0) 
        return (0);
    for (i = 0; i < CACHE_ST_NR ; i++) {
        if (tl_cache [i].blkno == blkno && tl_cache [i].blkno != (-1))  /* cached */
            goto  _BLK_CPY;
    }
    for (i = 0; i < CACHE_ST_NR ; i++) {
        if (tl_cache [i].blkno == (-1)) {/* find a free slot */
            tl_cache [i].blkno = blkno;
            break;
        }
    }
    if (i >= CACHE_ST_NR)     /* no more free tl_cache ,then make one */
        i = sync_tl_cache ();
    if (read_blk (tl_cache [i].buf,blkno)) 
        return (0);
_BLK_CPY:
    if (rw == READ) {
        src = (void*)tl_cache [i].buf + ((fblkno & NAND_BLK_MASK) << BLK_SIZE_SFT);
        to = buf;
    } else {
        to = (void*)tl_cache [i].buf + ((fblkno & NAND_BLK_MASK) << BLK_SIZE_SFT);
        src = buf;
        tl_cache [i].dirt = true;
    }
    memcpy (to,src,BLK_SIZE);
    return (BLK_SIZE);
}

i32 sys_sync_tl (void)
{
    i16 i = 0;

    for (i = 0 ; i < 8 ; i++) { 
        if (tl_cache [i].blkno != (-1) && tl_cache [i].dirt) {
            if (write_blk (tl_cache[i].blkno,(void*)tl_cache [i].buf))
                return (-1);
            tl_cache [i].dirt = false;
        }
    }
    return (0);
}
