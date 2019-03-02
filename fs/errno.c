#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <ecasey/fs.h>

LOCAL RO i8 *err_str [] = {
    "",
    "NO SUCH PATH OR DIR.",
    "PATH ALREADY EXIST.",
    "NO MORE SRC FREE.",
    "READ OPERATION FAILED.",
    "WRITE OPERATION FAILED.",
    "OPERATION NOT ALLOWED.",
    "OBJECT IS BUSY",
    "OBJECT IS FREE",
    "OPEN FAILED",
    "EMPTY ENTRY",      /* 10 */
    "NOT EMPTY ENTRY",
    "IS A DIR",
    "NOT A DIR.",
    "BAD FD",
    "INVALIATE OBJECT.",
    "NO FREE FD.",
    "BAD FS CALL",
    "NOT ELF FILE.",
    "OBJECT IS UNDEFED.",
    "OBJECT IS NOT SURPOTED YET.",
    "OBJECT IS FULL.",
};

void perr (i32 errno)
{
    if (errno >= 0)
        return ;
    errno = -errno;
    printf ("%s\n",err_str [errno % (sizeof(err_str))]);
}
