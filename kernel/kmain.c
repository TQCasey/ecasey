#include <2440/drvs.h>
#include <ecasey/kernel.h>
#include <assert.h>
#include <ecasey/pcb.h>
#include <2440/cache.h>
#include <sys/types.h>
#include <string.h>

PCB     *current_p = (void*)0;
extern  TASK *task_st[];
LOCAL   void  Idle (void);

extern u32  NR_TASKS ;
extern void restart (void);
extern void start_mmu (void);
extern void uart_i32r_conf (u32 prc);
extern void uart_recv (void);
extern i32  uart_printf (RO i8 *fmt,...);
extern i32  init_mmap (void);
extern void init_i32r_vector (void);
extern void init_task_st (void);
extern void init_timer (i32 nr);
extern void init_mail (void);
extern void rebuild_mmu (void);
extern void init_page_map (void);
extern void keys_i32r_conf (void);
extern void setup_syscalls (void);
extern void blk_drv_init (void);
extern void init_task (u32,u32);
extern i32  createp (RO i8*,i32,i32);

extern u32      tPCLK,		/* pclk */
                tHCLK,		/* hclk */
                tFCLK,		/* fclk */
                tCPU_FREQ,	/* cpu freq */
                tUPLL,		/* USB */
                tUCLK ;		/* USB CLOCK */

extern void init_intr_vector (void);
extern i8* get_date (i8 *buffer,u32 size);
extern void uart_intr_conf (u32 prc);
extern void keys_intr_conf (void);
extern void read_ramdisk (void);

void kernel_start (void)
{
    printk ( "$ > Yes,we are in kernel space now  !\n" );
    rebuild_mmu ();     
    printk ( "$ > MMU rebuilt done !  !\n" );

    save_cpu_bus_freq ();
    printk ("$ > CPU FRQ : %d MHZ ,PCLK : %d MHZ ,HCLK : %d MHZ ,UCLK : %d MHZ !\n",
             tCPU_FREQ/1000000,tPCLK/1000000,tHCLK/1000000,tUCLK/1000000);

    copy_nand_to_sdram (VECTOR_TBL,VECT_IMG,VECT_SIZE);   
    set_high_vector ();
    printk ( "$ > High vector init done !\n" );

    memset ((void*)KERNEL_SPACE_START,0,0x1000);
    init_page_map ();
    printk ( "$ > page map init done  !\n" );

    init_intr_vector ();
    printk ( "$ > vector interrupt table init done !\n");

    init_timer (0);
    printk ( "$ > Timer 0 init done !\n");

    i8 tm[40] = {0};
    get_date (tm,40);
    printk ( "$ > RTC : current time: %s\n",tm );

	keys_intr_conf ();
    printk ( "$ > Keys hit i32r init done !\n");

    init_task_st ();
    printk ( "$ > task structs init done !\n");

    setup_syscalls ();
    printk ( "$ > sys call tables init done !\n");

    uart_intr_conf ((u32)uart_recv);
    printk ( "$ > key hit input ok now ! !\n");

    lcd_init ();
    printk ( "$ > TFT init done !\n" );
    lcd_blush (0xFF2F);

#if 0
    touchpl_init ();
    printk ( "$ > Touch pannel  init done !\n" );
#endif
    blk_drv_init ();
    printk ( "$ > blk driver init done !\n" );

    /* init basic service tasks */
    init_task (KERNEL_TTB,(u32)&Idle);
    printk ( "$ > idle task init done !\n");

    printk ( "$ > pid = %d is created! \n",createp ("fs",4,0));
    printk ( "$ > pid = %d is created! \n",createp ("sh",4,0));

    printk ( "$ > read ramdisk image !\n" );
    read_ramdisk ();

    k_reenter = 1;
    current_p = task_st[0];
    restart ();                     /* MOVE TO SYSTEM MODE NOW !! */
}

void Idle (void)
{
    for (;;);               
}
