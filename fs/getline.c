#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <ecasey/exec.h>

i32 getargv (RO i8 *src,i8 argv[][NR_SRC_SIZE])
{
    i8 *p  = (i8*)src;
    i32 argc = 0,i = 0;

    while (*p) {
        while (*p && ((*p == ' ') || (*p == '\t'))) p ++;
        while (*p && (*p != ' ') && (*p != '\t')) argv[argc][i++] = *p ++;
        argv[argc][i] = '\0';
        i = 0;
        argc ++;
    }
    return (argc);
}
