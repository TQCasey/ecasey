#include <string.h>
#include <assert.h>

#include <2440/drvs.h>


void assert_fail (const char* expression,
                  const char* local_file,
                  const char* base_file,
                  unsigned long line_nr ) 
{
    lcd_printf (0,0xFFFF,0,"assert( %s ) failed!!!\nlocal file: %s,base_file: %s,line: %u",
            expression,local_file,base_file,line_nr )  ;
    /*should never arrive here!!!!!*/
    __asm__ ("b     .");
}

