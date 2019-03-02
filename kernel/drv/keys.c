#include <2440/drvs.h>
#include <assert.h>
#include <2440/drvs.h>
#include <ecasey/kernel.h>
#include <sys/types.h>

LOCAL 
void  key3_proc (void)
{
    printk ( "3" );
	rSRCPND	|= (1<<2);          /* this is must be first */
	rINTPND	|= (1<<2);          /* then do this line */
}

LOCAL 
void  key1_proc (void)
{
	rSRCPND	|= (1<<1);
	rINTPND	|= (1<<1);
}

LOCAL
void  key2_proc (void)
{
	rEINTPEND |= 1<<4;          /* eintpnd first */
	rSRCPND	|= (1<<4);          /* 2nd */
	rINTPND	|= (1<<4);          /* the last one to clean up */
}

/* ext int 0 */
LOCAL 
void  key4_proc (void)
{
	rSRCPND	|= (1<<0);
	rINTPND	|= (1<<0);
}

void keys_intr_conf (void)
{
	/* 0,1,2,4 ext_int conf */					    /* --which--*/
	rGPFCON 	&= ~((3<<0)|(3<<2)|(3<<4)|(3<<8));	/* clear */
	rGPFCON 	|= ((2<<0)|(2<<2)|(2<<4)|(2<<8));	/* set */
	
	/* 0,1,2,4 low level */						    /* --how-- */
	rEXTINT0 	&= ~((7<<0)|(7<<4)|(7<<8)|(7<<16)); /* clear */	
	rEXTINT0 	|= ((3<<0)|(3<<4)|(3<<8)|(3<<16));	/* set */
	
	rINTMOD		&= ~(0x17);						    /* what type */
	
	pISR_EINT0 	= (u32)key4_proc;			    /* --do what-- */
	pISR_EINT1	= (u32)key1_proc;	
	pISR_EINT2	= (u32)key3_proc;
	pISR_EINT4_7= (u32)key2_proc;

	/* pnd flags clear */						    /* stuck off */
	rSRCPND		|= 0x17;
	rINTPND		|= 0x17;
	rEINTPEND	|= (1<<4);
	
	/* mask */
	rINTMSK		&= ~0x17;						
	rEINTMASK	&= ~(1<<4);						    /* go go go */
}
