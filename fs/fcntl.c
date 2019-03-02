#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <ecasey/fs.h>
#include <errno.h>


extern i32 do_close (i32 pid,i32 fd);

LOCAL i32 dupfd (i32 pid,i32 fd,i32 start) 
{
    if ((fd >= MAX_FILPS) || !pfilp (pid,fd) || (start >= MAX_FILPS)) 
        return (-EBADFD);
    while (start < MAX_FILPS) {
        if (pfilp (pid,start))
            start ++;
        else 
            break;
    }
    if (start >= MAX_FILPS) 
        return (-ENOFD);
    (pfilp (pid,fd) = pfilp (pid,start))->f_cnt ++;
    return (start);
}

i32 do_dupfd (i32 pid,i32 fd)
{
    return (dupfd (pid,fd,0));
}

i32 do_dup2 (i32 pid,i32 old,i32 new)
{
    do_close (pid,new);
    return (dupfd (pid,old,new));
}
