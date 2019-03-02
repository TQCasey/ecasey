#ifndef __CACHE_H__
#define __CACHE_H__

#define dcache_off()       \
{\
    __asm__  __volatile__ (\
        "mrc    p15,0,r0,c1,c0,0\n"\
        "bic    r0,r0,#1<<2\n"\
        "mcr    p15,0,r0,c1,c0,0\n"\
        ::: "r0"\
    );\
}

#define dcache_on()       \
{\
    __asm__  __volatile__ (\
        "mrc    p15,0,r0,c1,c0,0\n"\
        "orr    r0,r0,#1<<2\n"\
        "mcr    p15,0,r0,c1,c0,0\n"\
        ::: "r0"\
    );\
}

#define icache_off()       \
{\
    __asm__  __volatile__ (\
        "mrc    p15,0,r0,c1,c0,0\n"\
        "bic    r0,r0,#1<<12\n"\
        "mcr    p15,0,r0,c1,c0,0\n"\
        ::: "r0"\
    );\
}

#define icache_on() \
{\
     __asm__  __volatile__ (\
        "mrc    p15,0,r0,c1,c0,0\n"\
        "orr    r0,r0,#1<<12\n"\
        "mcr    p15,0,r0,c1,c0,0\n"\
        ::: "r0"\
    );\
}

#define mmu_off()   \
{\
    __asm__  __volatile__ (\
        "mcr    p15,0,r0,c1,c0,0\n"\
        "bic    r0,r0,#1\n"\
        "mrc    p15,0,r0,c1,c0,0\n"\
        ::: "r0"\
    );\
}

#define mmu_on()    \
{\
    __asm__  __volatile__ (\
        "mrc    p15,0,r0,c1,c0,0\n"\
        "orr    r0,r0,#1\n"\
        "mcr    p15,0,r0,c1,c0,0\n"\
        ::: "r0"\
    );\
}

/* replacement way */
#define set_robin_replacement() \
{\
    __asm__  __volatile__ (\
        "mrc p15,0,r0,c1,c0,0\n"\
        "orr r0,r0,#1<<14\n"\
        "mcr p15,0,r0,c1,c0,0\n"\
        :::"r0"\
    );\
}


/* vector 0x00000 */
#define set_low_vector()    \
{\
    __asm__  __volatile__ (\
        "mrc p15,0,r0,c1,c0,0\n"\
        "bic r0,r0,#1<<13\n"\
        "mcr p15,0,r0,c1,c0,0\n"\
        :::"r0"\
    );\
}\

/* vector 0xFFFF0000 */
#define set_high_vector()    \
{\
    __asm__  __volatile__ (\
        "mrc p15,0,r0,c1,c0,0\n"\
        "orr r0,r0,#1<<13\n"\
        "mcr p15,0,r0,c1,c0,0\n"\
        :::"r0"\
    );\
}\

#define set_little_endian() \
{\
    __asm__  __volatile__ (\
        "mrc p15,0,r0,c1,c0,0\n"\
        "bic r0,r0,#1<<7\n"\
        "mcr p15,0,r0,c1,c0,0\n"\
        :::"r0"\
    );\
}

#define align_fault_on()    \
{\
    __asm__  __volatile__ (\
        "mrc p15,0,r0,c1,c0,0\n"\
        "orr r0,r0,#1<<1\n"\
        "mcr p15,0,r0,c1,c0,0\n"\
        :::"r0"\
    );\
}\


/*
 * arm920t has 16 domains ,one kind of map can use different 
 * domain access priority ,in ecasey just 3 : fault,sys and usr mode
 */


#define invalidate_icache() \
{\
    __asm__  __volatile__ (\
        "mov    r0,#0\n"\
        "mcr    p15,0,r0,c7,c5,0\n"\
        ::: "r0"\
    );\
}

#define invalidate_dcaches() \
{\
    __asm__  __volatile__ (\
        "mov    r0,#0\n"\
        "mcr    p15,0,r0,c7,c6,0\n"\
        ::: "r0"\
    );\
}

#define invalidate_dcache(index)   \
{\
     __asm__  __volatile__ (\
        "mcr    p15,0,r0,c7,c6,1\n"\
        :\
        :[i] "r"(index)\
    );\
}




#define invalidate_idcache()    \
{\
    __asm__  __volatile__ (\
        "mov    r0,#0\n"\
        "mcr    p15,0,r0,c7,c7,0\n"\
        :::"r0"\
    );\
}\



#define clr_invalidate_dcache(index)    \
{\
    __asm__  __volatile__ (\
        "mcr    p15,0,%[i],c7,c14,2\n"\
        :: [i]"r"(index)\
    );\
}


#define set_domain(domain) \
{\
     __asm__  __volatile__ (\
        "mcr    p15,0,%[dom],c3,c0,0\n"\
        :\
        :[dom] "r"(domain)\
    );\
}

#define fsce_off()          \
{\
    __asm__  __volatile__ (\
        "mov    r0,#0\n"\
        "mcr    p15,0,r0,c13,c0,0\n"\
    );\
}\


/* armv4t */
#define drain_wb()    \
{\
    __asm__  __volatile__ (\
        "mov    r0,#0\n"\
        "mcr    p15,0,r0,c7,c10,4\n"\
        :::"r0"\
    );\
}\

#define invalidate_tlbs()    \
{\
     __asm__  __volatile__ (\
        "mov    r0,#0\n"\
        "mcr    p15,0,r0,c8,c7,0\n"\
        :::"r0" \
    );\
}

#define set_ttb_base(ttb_base) \
{\
     __asm__  __volatile__ (\
        "mcr    p15,0,%[base],c2,c0,0\n"\
        :\
        :[base] "r"(ttb_base)\
    );\
}


#define invalidate_tlb(index)   \
{\
     __asm__  __volatile__ (\
        "mcr    p15,0,%[i],c8,c6,1\n"\
        :\
        :[i] "r"(index)\
    );\
}

#endif
