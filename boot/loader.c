#include <2440/2440addr.h>
#include <2440/drvs.h>
#include <2440/ports.h>

/* loader_start from here ! */
void loader_start (void)
{
    /* load kernel.bin to sdram*/
    copy_nand_to_sdram (LOADK_START,LOADER_IMG,LOADER_SIZE);
}
