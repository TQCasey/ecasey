#ifndef __DRVS_H__
#define __DRVS_H__

#include <2440/2440addr.h>
#include <sys/types.h>

/* led  */
extern void led_conf (void);
extern void led_opt (int led_nr,int val);

/* uart */
extern void use_uart (i32 uart);
extern void uart_conf (i32 uart,u32 pclk,u32 baud);
extern u8 uart_sendc (u8 ch);
extern RO i8 *uart_sends (RO i8 *s);
extern u8 uart_recvc (void);
extern i32  uart_printf (RO i8 *fmt,...);

/* 
 * NAND FLASH FOR TQ2440 compolents as follows :
 * K9F2G08U0A 256M 8BITS ,2K blks ,64 pages per block ,2K bytes per page 
 * 64 secondry bytes,so totally ,(2048 + 64) * 64 * 2048 bytes  
 */

#define BYTES_PER_PAGE		2048    /* 2K/page */
#define SP_BYTES_PER_PAGE	64      /* 64 secondry bytes */
#define PAGES_PER_BLK		64      /* 64 pages/block */
#define BLKS_PER_NANDF		2048    /* 2K blocks/nand flash */
#define MAX_BAD_BLKS        64      /* max bad blocks */

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
#define NF_WAIT_RB()		{while(!(rNFSTAT&(1<<2)));}		/* wait till RnB not busy */
#define NF_NOT_BUSY()       {while(!(rNFSTAT&(1<<0)));}     /* wait till nand flash not busy */

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

extern i32 nand_reset (void);
extern u32 copy_nand_to_sdram (u32 sdram_addr,u32 nand_addr,u32 img_size);

/* clock */
extern void save_cpu_bus_freq (void);

/* MMU TTB 0 BASE ATTR */
#define TTB0_FAULT          (0|(1<<4))          /* TTB FAULT */
#define TTB0_COARSE         (1|(1<<4))          /* COARSE PAGE BASE ADDR */
#define TTB0_SEG            (2|(1<<4))          /* SEG BASE ADDR */
#define TTB0_FINE           (3|(1<<4))          /* FINE PAGE BASE ADDR */

/* MMU TTB 1 BASE ATTR */ 
#define TTB1_FAULT          (0)
#define TTB1_LPG            (1)                 /* Large page */
#define TTB1_SPG            (2)                 /* small page */
#define TTB1_TPG            (3)                 /* tiny page */

/* domain access priority level */
#define FAULT_PL	        (0x0)               /* domain fault */
#define USR_PL	            (0x1)               /* usr mode */
#define RSV_PL              (0x2)               /* reserved */
#define SYS_PL	            (0x3)               /* sys mode */

#define DOMAIN_FAULT		(0x0<<5)            /* fault 0*/
#define DOMAIN_SYS		    (0x1<<5)            /* sys 1*/
#define DOMAIN_USR          (0x2<<5)            /* usr 2*/

/* C,B bit */
#define CB		            (3<<2)              /* cache_on, write_back */
#define CNB		            (2<<2)              /* cache_on, write_through */ 
#define NCB                 (1<<2)              /* cache_off,WR_BUF on */
#define NCNB		        (0<<2)              /* cache_off,WR_BUF off */

/* ap 2 bits */
#define AP_FAULT            (0<<10)             /* access deny */
#define AP_SU_ONLY          (1<<10)             /* rw su only */
#define AP_USR_RO		    (2<<10)             /* sup=RW, user=RO */
#define AP_RW		        (3<<10)             /* su=RW, user=RW */


/* page dir 1 ap0 */
#define AP0_FAULT           (0<<4)              /* FAULT */
#define AP0_SU_ONLY         (1<<4)              /* rw su only */
#define AP0_USR_RO		    (2<<4)              /* sup=RW, user=RO */
#define AP0_RW		        (3<<4)              /* su=RW, user=RW */

/* page dir 1 ap1 */
#define AP1_FAULT           (0<<6)              /* FAULT */
#define AP1_SU_ONLY         (1<<6)              /* rw su only */
#define AP1_USR_RO		    (2<<6)              /* sup=RW, user=RO */
#define AP1_RW		        (3<<6)              /* su=RW, user=RW */


/* page dir 1 ap2 */
#define AP2_FAULT           (0<<8)              /* FAULT */
#define AP2_SU_ONLY         (1<<8)              /* rw su only */
#define AP2_USR_RO		    (2<<8)              /* sup=RW, user=RO */
#define AP2_RW		        (3<<8)              /* su=RW, user=RW */

/* page dir 1 ap3 */
#define AP3_FAULT           (0<<10)             /* FAULT */
#define AP3_SU_ONLY         (1<<10)             /* rw su only */
#define AP3_USR_RO		    (2<<10)             /* sup=RW, user=RO */
#define AP3_RW		        (3<<10)             /* su=RW, user=RW */

#define AP_RW_ALL           ((3<<4)|(3<<6)|(3<<8)|(3<<10))
#define AP_USR_RO_ALL       ((2<<4)|(2<<6)|(2<<8)|(2<<10))
#define AP_RO_ALL           ((1<<4)|(1<<6)|(1<<8)|(1<<10))  
#define AP_FAULT_ALL        (0<<4)              /* all 1K is fault */



#define NORMAL_SEG_ATTR     (AP_RW | DOMAIN_USR | NCNB | TTB0_SEG)

extern void mmu_init (void);

/* LCD DRIVER */
#define LCD_WIDTH 			480			        /* width */
#define LCD_HEIGHT 			272			        /* height */
#define LCD_ASCII_X			8
#define LCD_ASCII_Y			16

/*  for lcd_con1 */
#define LCD_PIXCLK 			(4<<8)		        /* lcd pix clk */
#define LCD_MMODE			(0<<7)		        /* TFT clr */	
#define LCD_PNRMODE			(3<<5)		        /* TFT */
#define LCD_BPPMODE			(0x0C<<1)	        /* 16 bpp */
#define LCD_ENABLE			(1<<0)		        /* lcd enable */
#define LCD_DISABLE			(0<<0)		        /* lcd disable */

/* for lcd_con2 */
#define LCD_UPPER_MARGIN	(5<<24)		        /* upper margin lines */
#define LCD_LINE_VAL		((LCD_HEIGHT-1)<<14)/* line cnt */
#define LCD_LOWER_MARGIN	(3<<6)		        /* lower margin lines */	
#define LCD_VPWR_LEN		(1<<0)		        /* vertical pwr len */

/* for lcd_con3 */
#define LCD_LEFT_MARGIN		(10<<0)	            /* left margin lines */
#define LCD_COLS_VAL		((LCD_WIDTH-1)<<8)	/* cols cnt */
#define LCD_RIGHT_MARGIN	(39<<19)		    /* right margin lines */

/* for lcd_con4 */
#define LCD_HPWR_LEN		0			        /* horz pwr len */

/* for lcd_con5 */
#define LCD_BPP24BL			(0<<12)		        /* high 8 bits is invalidate */
#define LCD_FRM565			(1<<11)		        /* r:g:b = 5:6:5 */
#define LCD_INVVCLK			(0<<10)		        /* rising level invalidate */
#define LCD_INVVLINE		(1<<9)		        /* HSYNC sig invert	*/
#define LCD_INVFRAME		(1<<8)		        /* VSYNC sig invert */
#define LCD_INVVD			(0<<7)		        /* VD sig invert */
#define LCD_INVVDEN			(0<<6)		        /* VDEN sig invert */
#define LCD_INVPWREN		(0<<5)		        /* show PWREN sig stat*/
#define LCD_INVLEND			(0<<4)		        /* BSWP sig invert */
#define LCD_ENPWREN			(1<<3)		        /* PWREN sig enable */
#define LCD_ENLEND			(0<<2)		        /* LEND sig enabled */
#define LCD_BSWP			(0<<1)		        /* BYTE SWP enabled */
#define LCD_HSWP			(1<<0)		        /* HALF SWP enabled */

#define LCD_XSIZE  			LCD_WIDTH
#define LCD_YSIZE  			LCD_HEIGHT
#define SCR_XSIZE  			LCD_WIDTH
#define SCR_YSIZE  			LCD_HEIGHT


extern void lcd_init (void);
extern void lcd_blush (u16 c);
extern i32 lcd_printf (u32 c,u32 bk_c,u32 st,i8 *fmt,...);
extern void draw_map (i32 xstart,i32 ystart,i32 width,i32 height,RO u8 *bmp);
/* RTC */
#define BCD2O(bcd)	((bcd>>4)&0x0F)*10 + (bcd & 0x0F)
#define O2BCD(o)	(((o/10) << 4) | (o%10))

extern i8* get_date (i8 *buffer,u32 size);
extern void set_date (i32 year,i32 mon,i32 date,
			          i32 day,
			          i32 hour,i32 mini,i32 sec);

/* ADC */
#define PRSCVL  (24)

#endif
