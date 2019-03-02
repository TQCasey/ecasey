#include <2440/2440addr.h>
#include <ecasey/kernel.h>
#include <sys/types.h>

void led_conf (void)
{
	rGPBCON &= ~((3<<10)|(3<<12)|(3<<14)|(3<<16));	/* clear */
	rGPBCON |=  ((1<<10)|(1<<12)|(1<<14)|(1<<16));	/* set */
	/* pull up by default */
}

void led_opt (i32 led_nr,i32 val)
{
	led_nr &= 3;
	val = (val > 0);
	led_nr += 5;

	if (rGPBDAT & (1 << led_nr)) {	/* if on */
		if (!val)	/* turn off */
			rGPBDAT &= ~(1<<led_nr);	
	} else {		
		if (val) 	/* turn on */
			rGPBDAT |= (1<<led_nr);
	}
}
