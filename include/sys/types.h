#ifndef __TYPES_H__
#define __TYPES_H__

//#define __WIN32__

typedef          char       i8;
typedef          short      i16;
typedef          long       i32;
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned long       u32;

#ifdef __WIN32__
typedef unsigned __int64	u64;
typedef __int64				i64;
#else
typedef unsigned long long  u64;
typedef long long           i64;
#endif


typedef int                 bool;
#define false               0
#define true                1

#define LOCAL               static   
#define INLINE              inline
#define RO                  const
#define VAR                 volatile
#define __VAR__             __volatile__

#define high(x)             (unsigned char)(((unsigned short)(x))>>8)
#define low(x)              (unsigned char)(x)
#define size(x)             (sizeof(x))
#define getw(ptr)           (*(unsigned short*)(ptr))
#define getb(ptr)           (*(unsigned char*)(ptr))
#define getdw(ptr)          (*(unsigned long*)(ptr))
#define getdd(ptr)          (*(unsigned long long*)(ptr))



#endif
