#include <2440/2440addr.h>
#include <ecasey/kernel.h>
#include <string.h>
#include <sys/types.h>

extern RO u8 *uart_sends (RO u8 *s);

i32 panic (RO i8 *fmt,...)
{
    va_list ap;
    i32 i = 0 ;
    u8 string[1024+1] = {0};

    va_start(ap,fmt);
    i = vsprintf((i8*)string,fmt,ap);
    uart_sends(string);
    va_end(ap);

    __cli ();
    while(1);

    return (i);
}
