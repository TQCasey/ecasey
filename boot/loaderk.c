/* this is for low level init operations 
 * such as start mmu & map kernel space 
 * 3G - (3G + 4M),after this is done,it 
 * will load kernel to kernel space
 */

#include <2440/2440addr.h>
#include <2440/drvs.h>
#include <2440/ports.h>
#include <ecasey/kernel.h>
#include <2440/cache.h>

extern void start_mmu (void);

/* now it is time to start low level init */
void load_kernel (void)
{
    start_mmu ();                   /* set mmu */
    led_conf ();                    /* led conf */
    uart_conf (0,0,0);              /* init uart0 */

    printk ( "\n$ > Booting......\n" );
    printk ( "$ > mmu actived ! !\n" );

    led_opt (0,1);
    led_opt (1,1);
    led_opt (2,1);
    led_opt (3,1);                  /* all leds off */

    /* prepare 3G - (3G+2M) kernel code space */
    printk ( "$ > loading kernel ....\n" );
    copy_nand_to_sdram (KERNEL_CODE_START,
                        KERNEL_IMG,
                        KERNEL_IMG_SIZE);

    printk ( "$ > loading kernel done ! \n$ > Start to boot kernel ....\n" );
}
