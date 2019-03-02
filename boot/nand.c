/* this address must be spedified specially 
 * 'ause the mmu is not enabled ,and io address
 * is not in our space now !!!! No big problem !
 */
#include <sys/types.h>

#define rNFCONF             (*(volatile unsigned *)0x4E000000)		//NAND Flash configuration
#define rNFCONT             (*(volatile unsigned *)0x4E000004)      //NAND Flash control
#define rNFCMD              (*(volatile unsigned *)0x4E000008)      //NAND Flash command
#define rNFADDR             (*(volatile unsigned *)0x4E00000C)      //NAND Flash address
#define rNFDATA             (*(volatile unsigned *)0x4E000010)      //NAND Flash data
#define rNFDATA8            (*(volatile unsigned char *)0x4E000010) //NAND Flash data
#define NFDATA              (0x4E000010)                            //NAND Flash data address
#define rNFMECCD0           (*(volatile unsigned *)0x4E000014)      //NAND Flash ECC for Main Area
#define rNFMECCD1           (*(volatile unsigned *)0x4E000018)
#define rNFSECCD            (*(volatile unsigned *)0x4E00001C)		//NAND Flash ECC for Spare Area
#define rNFSTAT             (*(volatile unsigned *)0x4E000020)		//NAND Flash operation status
#define rNFESTAT0           (*(volatile unsigned *)0x4E000024)
#define rNFESTAT1           (*(volatile unsigned *)0x4E000028)
#define rNFMECC0            (*(volatile unsigned *)0x4E00002C)
#define rNFMECC1            (*(volatile unsigned *)0x4E000030)
#define rNFSECC             (*(volatile unsigned *)0x4E000034)
#define rNFSBLK             (*(volatile unsigned *)0x4E000038)		//NAND Flash Start block address
#define rNFEBLK             (*(volatile unsigned *)0x4E00003C)		//NAND Flash End block address

#define rGPACON             (*(volatile unsigned *)0x56000000)      //Port A control
#define rGPADAT             (*(volatile unsigned *)0x56000004)      //Port A data

#define BYTES_PER_PAGE		2048    /* 2K/page */
#define SP_BYTES_PER_PAGE	64      /* 64 secondry bytes */
#define PAGES_PER_BLK		64      /* 64 pages/block */
#define BLKS_PER_NANDF		2048    /* 2K blocks/nand flash */

#define NFCONF_INIT			0xF830  
#define NEED_EXT_ADDR		1

#define NF_READ1			0x00
#define NF_READ2            0x30    /* NAND FLASH READ */

#define NF_READ_CPY_BACK1   0x00    
#define NF_READ_CPY_BACK2   0X35    /* NAND FLASH READ COPY BACK */

#define NF_READ_ID          0x90    /* NAND FLASH READ ID */
#define NF_RESET            0xFF    /* NAND FLASH RESET */

#define NF_PAGE_WRITE1      0x80
#define NF_PAGE_WRITE2      0x10    /* NAND FLASH WRITE */

#define NF_RANDOM_WRITE     0x85    /* NAND FLASH RANDOM WRITE */

#define NF_RANDOM_READ1     0x05    
#define NF_RANDOM_READ2     0xE0    /* NAND FLASH RANDOM READ */

#define NF_READ_STATUS      0x70    /* NAND FLASH READ STATUS */

#define NF_BLK_ERASE1       0x60
#define NF_BLK_ERASE2       0xD0    /* NAND FLASH ERASE */

#define STATUS_READY		0x40	/*  Ready */
#define STATUS_ERROR		0x01	/*  Error */

#define NF_CMD(cmd)			{rNFCMD  = (cmd);}				/* cmd */
#define NF_ADDR(addr)		{rNFADDR = (addr);}				/* addr */
#define NF_WRDATA(data)		{rNFDATA = (data); }			/* data */

#define NF_nFCE_L()			{rNFCONT &= ~(1<<1);}			/* CE ON */
#define NF_nFCE_H()			{rNFCONT |= (1<<1);}			/* CE OFF */

#define NF_CLR_RB()		    {rNFSTAT |= (1<<2); }			/* clear RnB */
#define NF_WAIT_RB()		{while(!(rNFSTAT&(1<<2)));}		/* detect RnB when after write */

#define NF_RSTECC()			{rNFCONT |= (1<<4);}
#define NF_RDMECC0()		(rNFMECC0)
#define NF_RDMECC1()		(rNFMECC1)
#define NF_RDSECC()			(rNFSECC)
#define NF_RDDATA()			(rNFDATA)
#define NF_RDDATA8()		(rNFDATA8)

#define NF_WAITRB()			{while(!(rNFSTAT&(1<<0)));} 

#define NF_MECC_Unlock()	{rNFCONT &= ~(1<<5); }
#define NF_MECC_Lock()		{rNFCONT |= (1<<5); }
#define NF_SECC_Unlock()	{rNFCONT &= ~(1<<6); }
#define NF_SECC_Lock()		{rNFCONT |= (1<<6); }

#define	nand_read_byte()	(rNFDATA8)	
#define nand_write_byte(ch) ((rNFDATA8 = ch))

#define pNFCONF				rNFCONF 
#define pNFCMD				rNFCMD  
#define pNFADDR				rNFADDR 
#define pNFDATA				rNFDATA 
#define pNFSTAT				rNFSTAT 
#define pNFECC				rNFECC0  

#define NF_CE_ON()			NF_nFCE_L()
#define NF_CE_OFF()			NF_nFCE_H()
#define NF_DATA_R()			rNFDATA
#define NF_ECC()			rNFECC0

/* 100M HCLCK */
#define TACLS				1	                /* 1-clk(0ns) */
#define TWRPH0				4	                /* 3-clk(25ns) */
#define TWRPH1				0	                /* 1-clk(10ns)  TACLS+TWRPH0+TWRPH1>=50ns */

/* nand flash reset */
i32 nand_reset (void)
{
    NF_CE_ON ();                        /* chip on */
    NF_CLR_RB ();                       /* clear R/B */
    NF_CMD (NF_RESET);                  /* RESET */
    NF_WAIT_RB ();                      /* wait till not busy */
    NF_CE_OFF ();                       /* chip off */
    return (0);
}

u32 ll_nand_read_page (void *to,u32 start_addr)
{
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
    return (0);
}

/* init only once */
void nand_init (void)
{
	/* GPA [22:17] Init */
	rGPACON	&= ~(0x3f<<17);
	rGPACON |= (0x3F<<17);
	rNFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4)|(0<<0);
	rNFCONT = ((1<<4)|(1<<1)|(1<<0));	/* enable NANDFLASH */
	rNFSTAT = 0;		                /* clear NF_state */
	nand_reset();	
}
/* read imgs from nand flash to SDRAM ...
 * nand_addr will be 2K aligment ,so pages nr is just fine here !!! 
 *                                                      -- casey 
 */
u32 copy_nand_to_sdram (u32 sdram_addr,u32 nand_addr,u32 img_size)
{
    u32 pages = ((img_size&0x7FF) != 0) + (img_size>>11),i = 0;

    for (i = 0 ; i < pages ; i ++,sdram_addr += 2048 )
        ll_nand_read_page ((void*)sdram_addr,(nand_addr >> 11) + i);
    return (0);
}
