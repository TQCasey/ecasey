#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FILE_MODE           (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define BYTES_PER_PAGE		(2048)      /* 2K/page */
#define SP_BYTES_PER_PAGE	(64)        /* 64 secondry bytes */
#define PAGES_PER_BLK		(64)        /* 64 pages/block */
#define BLKS_PER_NANDF		(2048)      /* 2K blocks/nand flash */

int main (int argc,char *argv[])
{
    int fdin,i,j,k;
    char buf[2048] = {0};
    memset (buf,0xFF,sizeof(buf));

    if (argc < 2) {
        fprintf (stderr,"usage %s <filename> !\n",argv[0]);
        exit (1);
    }

    if ((fdin = open(argv[1],O_RDWR|O_CREAT|O_TRUNC,FILE_MODE)) < 0){
        printf("can't open %s for writing mode !", argv[1]);
        exit(1);
    }

    for (i = 0; i < BLKS_PER_NANDF ;i ++) {     /* blks */
        for (j = 0 ;j < PAGES_PER_BLK; j ++)    /* pages */
            write (fdin,buf,sizeof(buf));
    }

    close (fdin);

    return (0);
}
