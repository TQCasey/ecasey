#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <ecasey/fs.h>

struct proc_st process = {
    {ROOT_DEV,ROOT_IND_NR},
    {ROOT_DEV,ROOT_IND_NR},
    "/",
    0
},*proc = &process;


