#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>

i32 crash (RO i8 *fmt,...) 
{
    va_list va_p = (void*)0;
    i8 buf[1024] = {"crash : "};
    i32 len ;

    va_start (va_p,fmt);
    len = vsprintf (buf+8,fmt,va_p);
    va_end (va_p);

    printf (buf);
    while (1);
    return (len);
}



i32 warn (RO i8 *fmt,...)
{
    va_list va_p = (void*)0;
    i8 buf[1024] = {"warn : "};
    i32 len ;

    va_start (va_p,fmt);
    len = vsprintf (buf+7,fmt,va_p);
    va_end (va_p);

    printf (buf);
    return (len);
}
