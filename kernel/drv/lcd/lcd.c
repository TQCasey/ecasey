#include <2440/drvs.h>
#include <stdarg.h>
#include <string.h>
#include <ecasey/kernel.h>
#include <sys/types.h>

LOCAL void lcd_port_init (void)
{
    rGPCCON = 0xAAAA02A8;
    rGPCUP 	= 0xFFFFFFFF;	/* not pull up */

    rGPDCON = 0xAAAAAAAA;	
    rGPDUP	= 0xFFFFFFFF;	/* not pull up */
}
LOCAL void lcd_mode_init (void)
{
    rLCDCON1 = LCD_PIXCLK | LCD_MMODE | LCD_PNRMODE |LCD_BPPMODE | LCD_DISABLE;
    rLCDCON2 = LCD_UPPER_MARGIN | LCD_LINE_VAL | LCD_LOWER_MARGIN | LCD_VPWR_LEN ;
    rLCDCON3 = LCD_LEFT_MARGIN | LCD_COLS_VAL | LCD_RIGHT_MARGIN ;
    rLCDCON4 = LCD_HPWR_LEN ;
    rLCDCON5 = LCD_BPP24BL | LCD_FRM565 | LCD_INVVCLK | LCD_INVVLINE 
                | LCD_INVFRAME | LCD_INVVD | LCD_INVVDEN | LCD_INVPWREN 
                | LCD_INVLEND | LCD_ENPWREN | LCD_ENLEND | LCD_BSWP | LCD_HSWP;
}
#define	M5D(n)	    ((n)&0x1fffff)
#define LCD_BUF     (v2p(LCD_BUFFER))
typedef     u16 (*PLCD_BUF)[LCD_XSIZE];
LOCAL       PLCD_BUF    lcd_buf;

LOCAL void lcd_buff_init (void)
{
    lcd_buf = (PLCD_BUF)(LCD_BUFFER);

 	rLCDSADDR1 = ((LCD_BUF>>22)<<21) | ((M5D(LCD_BUF>>1))<<0);
 	rLCDSADDR2 = M5D((LCD_BUF + LCD_WIDTH * LCD_HEIGHT * 2)>>1);
 	rLCDSADDR3 = LCD_WIDTH;
}
 
void lcd_init (void)
{
    lcd_port_init ();
    lcd_mode_init ();
    lcd_buff_init ();

    rLCDINTMSK |= 0x03;		/* mask lcd sub intr */
    rTCONSEL &= ~7;			/* lpc3804 disabled */
    rTPAL 	= 0x00;			/* diable template */
    rTCONSEL &= ~((1<<4)|1);/* disable LPC3600/LCC3600 MODE */
    rLCDCON1 |= 0x01;		/* lcd on */

}

void lcd_blush( u16 c)
{
    u32 x,y ;
		
    for( y = 0 ; y < SCR_YSIZE ; y++ ){
    	for( x = 0 ; x < SCR_XSIZE ; x++ )
			lcd_buf[y][x] = c ;
    }
}

void draw_map (i32 xstart,i32 ystart,i32 width,i32 height,RO u8 *bmp)
{
    i32 x,y,c;
    i32 i = 0;
	
    for( y = 0 ; y < height ; y++ ){
		for( x = 0 ; x < width ; x++ ){
			c = bmp[i+1] | (bmp[i]<<8) ;	/* big endian */
			if ( ( (xstart+x) < SCR_XSIZE) && ( (ystart+y) < SCR_YSIZE) )
				lcd_buf[ystart+y][xstart+x] = c ;
			i += 2;
		}
    }
}

void put_pix (u32 x,u32 y,u32 c)
{
	if ( ( x < SCR_XSIZE) && ( y < SCR_YSIZE) )
		lcd_buf[y][x] = c ;
}


extern u8 __VGA[];
void lcd_put_ascii(u32 x,u32 y,u8 ch,u32 c,u32 bk_c,u32 st)
{
   u16 i,j;
   u8 *pZK,mask,buf;

   pZK = &__VGA[ch*16];
   for( i = 0 ; i < 16 ; i++ ) {
        mask = 0x80;
        buf = pZK[i];
        for( j = 0 ; j < 8 ; j++ ) {
            if( buf & mask )
                put_pix(x+j,y+i,c);
             else
                if( !st )
                    put_pix(x+j,y+i,bk_c);
             mask = mask >> 1;
        }
   }
}

LOCAL u32  pix_pos = 0;

i32 lcd_putch (u32 c,u32 bk_c,u32 st,i8 ch)
{
    switch (ch)
    {
        case '\n':
            pix_pos = (pix_pos / LCD_XSIZE + 1) * LCD_XSIZE;
            break;
        case '\t':
            pix_pos += 4 * LCD_ASCII_X;
            break;
        case '\b':
            pix_pos -= LCD_ASCII_X;
            break;
        default:
            lcd_put_ascii (pix_pos%LCD_XSIZE,(pix_pos/LCD_XSIZE)*LCD_ASCII_Y,ch,c,bk_c,st);
            pix_pos += LCD_ASCII_X;
            break;
    }
    return (ch);
}

i32 lcd_printf (u32 c,u32 bk_c,u32 st,i8 *fmt,...)
{
    va_list ap;
    i32 i = 0 ;
    i8 string[1024] = {0},*p = 0;
    p = string;

    va_start(ap,fmt);
    i = vsprintf(string,fmt,ap);

    while (*p)
        lcd_putch (c,bk_c,st,*p++);
    va_end(ap);
    return (i);
}

u32 lcd_gotoxy (i32 x,i32 y)
{
    pix_pos = (x * LCD_ASCII_X + y * LCD_XSIZE );
    return (pix_pos);
}
