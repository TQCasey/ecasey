#include <2440/2440addr.h>
#include <ecasey/kernel.h>
#include <sys/types.h>

u32  	    tPCLK,		/* pclk */
			tHCLK,		/* hclk */
			tFCLK,		/* fclk */
			tCPU_FREQ,	/* cpu freq */
			tUPLL,		/* USB */
			tUCLK ;		/* USB CLOCK */

/* do nothing but just save PCLK,HCLK,FCLK */
void save_cpu_bus_freq (void)
{
	u32 val;
	u8 m, p, s;

    val = rMPLLCON;					/* get mpll */
	m = (val>>12)&0xff;				/* get mdiv */
	p = (val>>4)&0x3f;				/* get pdiv */
	s = val&3;						/* get sdiv */
	/* the relationship of FCLK and Fin 
	 * m = mdiv + 8,p = pdiv + 2,s = sdiv 
	 * FCLK = 2 * m * Fin / ( p * 2 ^ s )
	 */
	m += 8;
	p += 2;
    tFCLK = (2*m*(FIN/100))/(p*(1<<s))*100;     
									            /* FCLK=400M  FIN=12M */
	val = rCLKDIVN;
	m = (val>>1)&3;					            /* hdivn */
	p = val&1;	
	val = rCAMDIVN;					            /* Pdivn */
	s = val>>8;
	switch (m) 						            /* hdivn */			
	{
	case 0:	tHCLK = tFCLK;	    break;	        /* FCLK / 1 */
	case 1:	tHCLK = tFCLK>>1;	break;	        /* FCLK / 2 */
	case 2:
			if(s&2) tHCLK = tFCLK>>3;	        /* FCLK / 8 when CAMDIVN[9] == 0 */
			else	tHCLK = tFCLK>>2;	        /* FCLK / 4 when CAMDIVN[9] == 1 */
							    break;
	case 3:
			if(s&1)	tHCLK = tFCLK/6;	        /* FCLK / 6 when CAMDIVN[9] == 0 */
			else	tHCLK = tFCLK/3;	        /* FCLK / 3 when CAMDIVN[9] == 1 */
							    break;
	}
	/* pdivn */
	tPCLK = (0 == p) ? (tHCLK) : (tHCLK>>1);
	/* DVS */
	tCPU_FREQ = (tHCLK) ? (tFCLK) : (s&0x10);
	val = rUPLLCON;
	m = (val>>12)&0xff;
	p = (val>>4)&0x3f;
	s = val&3;
	/* UPLL = (m + 8) * Fin / ((p+2) * (1<<s)) */
	tUPLL = ((m+8)*FIN)/((p+2)*(1<<s));
	/* DIVN_UPLL */
	tUCLK = ((rCLKDIVN>>3)&0xFF) ? (tUPLL>>1) : tUPLL;
}
