/*
 * FOR TQ2440 Low Level Inint 
 * BY Casey 2013 - 04 - 07
 */

#include <asm/ioaddr.h>
#include <2440/ports.h>

    .text
    .global _start
    .extern loader_start,nand_init,load_kernel
    .code   32
_start:
    b       reset                       /* reset */
    b       _undef_intstruct            /* undefined instruction */
    b       _software_intr              /* software interrupt */
    b       _fetch_abt                  /* fetch abort */
    b       _data_abt                   /* date abort */
    b       _reserved                   /* resereved */
    b       _irq                        /* irq */
    b       _fiq                        /* fiq */
    

reset:
    msr     cpsr_c,#SVC_MODE|NO_INTR    /* enter SVC mode */
	ldr		sp,	=1024*4                 /* stack 4K (stepping stone tail*/

    ldr     r0,=WTCON                   /* disbale watch dog */
    ldr     r1,=0x00
    str     r1,[r0]         

    ldr     r0,=INTMSK                  /* disbale ALL INT (main)*/
    ldr     r1,=0xFFFFFFFF
    str     r1,[r0]

    ldr     r0,=INTSUBMSK               /* disbale INT (sub) */
    ldr     r1,=0x7FFF          
    str     r1,[r0]         

    ldr     r0,=LOCKTIME                /* default lock time */
    ldr     r1,=0xFFFFFFFF    
    str     r1,[r0]         

    ldr     r0,=UPLLCON                 /* configure UPLL,if set UPLL MPLL ,set UPLL first*/
    ldr     r1,=((56<<12)|(2<<4)|(1<<0))
    str     r1,[r0]             

	ldr		r0,=MPLLCON			        /* */
	ldr		r1,=((92<<12)+(1<<4)+1)				
    str     r1,[r0]
                                        /* Fin = 12.0MHz, FCLK = 400MHz */

    ldr     r0,=CLKDIVN                 /* configure CLK DIVN */
    ldr     r1,=0x05                    /* FCLK : HCLK : PCLK 1:4:8*/
    str     r1,[r0]             

	mrc	    p15,0,r1,c1,c0,0            /* if hdivn is not 0 */
	orr 	r1,r1,#0xc0000000           /* set_async_busmode */
	mcr	    p15,0,r1,c1,c0,0	
	
    nop                                 /* about 7 nops */
    nop                         
    nop
    nop
    nop
    nop

    bl      memsetup                    /* mem configure */
    bl      nand_init                   /* nand flash init */
	bl		loader_start                /* call loader_start () */
    ldr     lr,=_endload                /* set return address */
    ldr     pc,=load_kernel             /* jmp to kernel loader */
_endload:
    ldr     pc,=KERNEL_CODE_START       /* jmp to kernel */
    b       .                           /* should not be here ! */

/* memsetup () to use outside SDRAM for TQ2440*/
memsetup:
    ldr     r1,=BWSCON                  /* mem control base (dest)*/
    ldr     r0,=mem_cfg_val             /* settng val (src) */
    ldr     r2,=13                      /* 13 regs */
.load:                                  /* i = 0,j = 0 */
    ldr     r4,[r0],#4                  /* r4 = mem_cfg_val[i ++] */
    str     r4,[r1],#4                  /* mem_con [j++] = r4 */
    subs    r2,r2,#1                    /* cnt -- */
    cmp     r2,#0                       /* if (cnt)*/ 
    bgt     .load                       /*  load */
    mov     pc,lr                       /* return */

/*
 * when exception occurs ,ARM will store next  exception
 * address to lr_xxx,and store cpsr to spsr_XXX and then
 * set  cpsr and force to enter in XXX mode & arm mode 
 * set pc = ISR_PROC_ADDRESS !,so when it comes here 
 * sp is the sp_XXX ！！！ 
 */

.macro      handle  irq_nr
    ldr     r13,=IRQ_TABLE              /* get irq_table */
    add     r13,r13,#\irq_nr            /* add offset */
    ldr     r13,[r13]                   /* load to r13 */
    mov     pc,r13                      /* jmp  to (irq_table+0x24) */
.endm

    .code 32
_undef_intstruct:                       /* undefined intruction (offset = 4) */
    handle  4
_software_intr:                         /* software interrupt (offset = 8) */
    handle  8
_fetch_abt:                             /* fectch abort (offset = 12) */
    handle  12
_data_abt:                              /* date abort (offset = 16) */
    handle  16
_reserved:                              /* reserved for sys (offset = 20) */
    handle  20
_irq:                                   /* irq (offset = 24) */
    handle  24
_fiq:                                   /* fiq (offset = 28) */
    handle  28

/*
 * memory configuration should be optimized for best performance
 * the following parameter is not optimized.
 * memory access cycle parameter strategy
 * 1. the memory settings is  safe parameters even at HCLK=75Mhz.
 * 2. SDRAM refresh period is for HCLK<=75Mhz.
 */

mem_cfg_val:
    .long   (0 | DW_B1 | DW_B2 | DW_B3 | DW_B4 | DW_B5 | DW_B6 | DW_B7)             /* BWSCON   */
    .long   (B0_Tacs | B0_Tcos | B0_Tacc | B0_Tcoh | B0_Tah | B0_Tacp | B0_PMC)     /* BANK 0   */
    .long   (B1_Tacs | B1_Tcos | B1_Tacc | B1_Tcoh | B1_Tah | B1_Tacp | B1_PMC)     /* BANK 1   */
    .long   (B2_Tacs | B2_Tcos | B2_Tacc | B2_Tcoh | B2_Tah | B2_Tacp | B2_PMC)     /* BANK 2   */
    .long   (B3_Tacs | B3_Tcos | B3_Tacc | B3_Tcoh | B3_Tah | B3_Tacp | B3_PMC)     /* BANK 3   */
    .long   (B4_Tacs | B4_Tcos | B4_Tacc | B4_Tcoh | B4_Tah | B4_Tacp | B4_PMC)     /* BANK 4   */
    .long   (B5_Tacs | B5_Tcos | B5_Tacc | B5_Tcoh | B5_Tah | B5_Tacp | B5_PMC)     /* BANK 5   */
    .long   (B6_MT | B6_Trcd | B6_SCAN)                                             /* BANK 6   */
    .long   (B7_MT | B7_Trcd | B7_SCAN)                                             /* BANK 7   */
    .long   (REFEN | TREFMD | Trp | Tchr |  Tsrc)                                   /* REFRESH  */
    .long   (BURST_EN | SCKE_EN | SCLK_EN | BK6_7_MAP)                              /* BANK SIZE*/
    .long   (WBL | TM | CL | BT | BL)                                               /* SMRSR6   */
    .long   (WBL | TM | CL | BT | BL)                                               /* SMRSR7   */
