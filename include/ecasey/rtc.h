#ifndef __RTC_H__
#define __RTC_H__

typedef struct _systime {
    char    year;
    char    mon;
    char    date;
    char    day;            /* 1 - 7 */
    char    hour;
    char    min;
    char    sec;
}systm_st;

#endif 
