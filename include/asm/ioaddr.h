#ifndef __IO_H__
#define __IO_H__

/* 
 * NOTE : 
 * THESE VALUE CAN BE USED JUST AT BOOT LOADER STAGE1 
 */

/* SDRAM CONTROLLER */
#define BWSCON              0x48000000          /* Bus width & wait status */
#define BANKCON0            0x48000004          /* Boot ROM control */
#define BANKCON1            0x48000008          /* BANK1 control */
#define BANKCON2            0x4800000c          /* BANK2 control */
#define BANKCON3            0x48000010          /* BANK3 control */
#define BANKCON4            0x48000014          /* BANK4 control */
#define BANKCON5            0x48000018          /* BANK5 control */
#define BANKCON6            0x4800001c          /* BANK6 control */
#define BANKCON7            0x48000020          /* BANK7 control */
#define REFRESH             0x48000024          /* DRAM/SDRAM refresh */
#define BANKSIZE            0x48000028          /* Flexible Bank Size */
#define MRSRB6              0x4800002c          /* Mode register set for SDRAM Bank6 */
#define MRSRB7              0x48000030          /* Mode register set for SDRAM Bank7 */

/* BWSCON configure value*/
#define DW_B1               (1<<4)              /* 16 bits for BANK1 (FLASH)*/
#define DW_B2               (1<<8)              /* 16 bits for BANK2 (PCMCIA)*/
#define DW_B3               (1<<12)             /* 16 bits for BANK3 (Ethernet)*/
#define DW_B4               (2<<16)             /* 32 bits for BANK4 (intel strata)*/
#define DW_B5               (1<<20)             /* 16 bits for BANK5 (A400)*/
#define DW_B6               (2<<24)             /* 32 bits for BANK6 (SDRAM)*/
#define DW_B7               (2<<28)             /* 32 bits for BANK7 (N.C) */

/*BANK0 CON */
#define B0_Tacs             (3<<13)             /* 4 clk */
#define B0_Tcos             (3<<11)             /* 4 clk */
#define B0_Tacc             (7<<8)              /* 14 clk */
#define B0_Tcoh             (3<<6)              /* 4 clk */
#define B0_Tah              (3<<4)              /* 4 clk */
#define B0_Tacp             (1<<2)              /* 3 clk */
#define B0_PMC              (0<<0)              /* normal */

/*BANK1 CON */
#define B1_Tacs             (1<<13)             /* 1 clk */
#define B1_Tcos             (1<<11)             /* 1 clk */
#define B1_Tacc             (6<<8)              /* 10 clk */
#define B1_Tcoh             (1<<6)              /* 1 clk */
#define B1_Tah              (1<<4)              /* 1 clk */
#define B1_Tacp             (0<<2)              /* 2 clk */
#define B1_PMC              (0<<0)              /* normal */

/*BANK2 CON */
#define B2_Tacs             (1<<13)             /* 1 clk */
#define B2_Tcos             (1<<11)             /* 1 clk */
#define B2_Tacc             (6<<8)              /* 10 clk */
#define B2_Tcoh             (1<<6)              /* 1 clk */
#define B2_Tah              (1<<4)              /* 1 clk */
#define B2_Tacp             (0<<2)              /* 2 clk */
#define B2_PMC              (0<<0)              /* normal */

/* BANK3 CON */
#define B3_Tacs             (1<<13)             /* 1 clk */
#define B3_Tcos             (1<<11)             /* 1 clk */
#define B3_Tacc             (6<<8)              /* 10 clk */
#define B3_Tcoh             (1<<6)              /* 1 clk */
#define B3_Tah              (1<<4)              /* 1 clk */
#define B3_Tacp             (0<<2)              /* 2 clk */
#define B3_PMC              (0<<0)              /* normal */

/* BANK4 CON */
#define B4_Tacs             (1<<13)             /* 1 clk */
#define B4_Tcos             (1<<11)             /* 1 clk */
#define B4_Tacc             (6<<8)              /* 10 clk */
#define B4_Tcoh             (1<<6)              /* 1 clk */
#define B4_Tah              (1<<4)              /* 1 clk */
#define B4_Tacp             (0<<2)              /* 2 clk */
#define B4_PMC              (0<<0)              /* normal */

/* BANK5 CON */
#define B5_Tacs             (1<<13)             /* 1 clk */
#define B5_Tcos             (1<<11)             /* 1 clk */
#define B5_Tacc             (6<<8)              /* 10 clk */
#define B5_Tcoh             (1<<6)              /* 1 clk */
#define B5_Tah              (1<<4)              /* 1 clk */
#define B5_Tacp             (0<<2)              /* 2 clk */
#define B5_PMC              (0<<0)              /* normal */

/* BANK6 CON SDRAM */
#define B6_MT               (3<<15)             /* SDRAM */
#define B6_Trcd             (1<<2)              /* 3 clk */
#define B6_SCAN             (1<<0)              /* 9 bits */

/* BANK7 CON SDRAM */
#define B7_MT               (3<<15)             /* SDRAM */
#define B7_Trcd             (1<<2)              /* 3 clk */
#define B7_SCAN             (1<<0)              /* 9 bits */

/* REFRESH CON */
#define REFEN               (1<<23)             /* SDRAM refresh enable */
#define TREFMD              (0<<22)             /* SDRAM CBR/AUTO REFRESH */
#define Trp                 (1<<20)             /* SDRAM RAS pre-change 3 clk */
#define Tsrc                (1<<18)             /* SDRAM semi ROW cycle time  5 clk ,Trc = Tsrc + Trp */
#define Tchr                (2<<16)             /* 2440 not used */
#define REF_CNTR            (1268)              /* 100M HCLK,7.8us */

/* BANKSIZE CON */
#define BURST_EN            (0<<7)              /* disable BURST */
#define SCKE_EN             (1<<5)              /* SDARM pwr down mode enable */
#define SCLK_EN             (1<<4)              /* SDRAM always active */
#define BK6_7_MAP           (2<<0)              /* BANK6/BANK7 : 128/128 */

/*SDRAM MODE REG SET REG     */
#define WBL                 (0<<9)              /* write burst len fixed */
#define TM                  (0<<7)              /* Test mode fixed */
#define CL                  (3<<4)              /* CAS latency 3 clks */
#define BT                  (0<<3)              /* burst type fixed */
#define BL                  (0<<0)              /* burst len fixed */

/* CLOCK & POWER MAN */
#define LOCKTIME            0x4c000000          /* PLL lock time counter */
#define MPLLCON             0x4c000004          /* MPLL Control */
#define UPLLCON             0x4c000008          /* UPLL Control */
#define CLKCON              0x4c00000c          /* Clock generator control */
#define CLKSLOW             0x4c000010          /* Slow clock control */
#define CLKDIVN             0x4c000014          /* Clock divider control */

/* INTERRUPT */
#define SRCPND              0x4a000000          /* Interrupt request status */
#define INTMOD              0x4a000004          /* Interrupt mode control */
#define INTMSK              0x4a000008          /* Interrupt mask control */
#define PRIORITY            0x4a00000c          /* IRQ priority control */
#define INTPND              0x4a000010          /* Interrupt request status */
#define INTOFFSET           0x4a000014          /* Interruot request source offset */
#define SUSSRCPND           0x4a000018          /* Sub source pending */
#define INTSUBMSK           0x4a00001c          /* Interrupt sub mask */

/* LED CONF */
#define GPFCON              0x56000050          /* Port F control */
#define GPFDAT              0x56000054          /* Port F data */
#define GPFUP               0x56000058          /* Pull-up control F */

/* Miscellaneous registe    r */
#define MISCCR              0x56000080          /* Miscellaneous control */
#define DCKCON              0x56000084          /* DCLK0/1 control */
#define EXTINT0             0x56000088          /* External interrupt control register 0 */
#define EXTINT1             0x5600008c          /* External interrupt control register 1 */
#define EXTINT2             0x56000090          /* External interrupt control register 2 */
#define EINTFLT0            0x56000094          /* Reserved */
#define EINTFLT1            0x56000098          /* Reserved */
#define EINTFLT2            0x5600009c          /* External interrupt filter control register 2 */
#define EINTFLT3            0x560000a0          /* External interrupt filter control register 3 */
#define EINTMASK            0x560000a4          /* External interrupt mask */
#define EINTPEND            0x560000a8          /* External interrupt pending */
#define GSTATUS0            0x560000ac          /* External pin status */
#define GSTATUS1            0x560000b0          /* Chip ID(0x32440000) */
#define GSTATUS2            0x560000b4          /* Reset type */
#define GSTATUS3            0x560000b8          /* Saved data0(32-bit) before entering POWER_OFF mode */
#define GSTATUS4            0x560000bc          /* Saved data1(32-bit) before entering POWER_OFF mode */
#define MSLCON              0x560000cc          /* Memory sleep control register */

/* WATCH DOG */
#define WTCON               0x53000000          /* Watch-dog timer mode */
#define WTDAT               0x53000004          /* Watch-dog timer data */
#define WTCNT               0x53000008          /* Eatch-dog timer count */

#endif

