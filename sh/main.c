#include <ecasey/kernel.h>
#include <ecasey/fs.h>
#include <ecasey/mail.h>
#include <stdio.h>
#include <ecasey/sysno.h>
#include <ecasey/rtc.h>
#include <sys/types.h>


void delay_sec (int sec);

int main (void)
{
    //char p[1024];
//    unsigned short x,y;

    while (1)
    {
        delay_sec (1);
        //getpos (&x,&y);
        //printf ( "x = %d,y = %d\n",x,y );
//        sendm (TASK_FS,FS_OPEN,p,sizeof(p));
  //      printf ( "FS tick = %d \n",getdw(p));
    }

    for (;;);
}
