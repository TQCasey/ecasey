/*
 * this file.c is used for kernel && user space funcs 
 * these funcs can in kernel or user space 
 * 2012 - 4 - 15  By casey 
 */
#include <stdarg.h>
#include <string.h>


void raise (int sig)
{
    ;
}

char *strcpy (char *dest,const char *src)
{ 
    char *p = dest ,*q = (char*)src; 
	while ((*p ++ = *q ++)) ;
	return (dest);
}

char *strcat (char* dest,const char *src)
{
	char *p = dest;
	strcpy (p + strlen(dest),src);
	
	return (p);	//copy
}

int atoi (const char *src) 
{
	int decem = 0 ;
    while (*src) {
        if (*src <= '9' && *src >= 0) 
            decem = decem * 10 + (*src++ - '0');
        else return (0);
    }

	return (decem) ;
}

int strncmp (const char *str1,const char *str2,int len) 
{
    if (!str1 || !str2 || !len) 
        return (-1);
    while (-- len && (*str1 == *str2) && *str1 && *str2) 
        str1 ++ ,str2 ++ ;
    return (*str1 - *str2) ;
}

char* strncpy (char *dest,const char *src,int len) 
{
    char *tmp = dest;
    while (len -- && (*dest ++  = *src ++ ));

    return (tmp);
}

int strcmp (const char *str1,const char *str2) 
{
    if (!str1 || !str2) 
        return (-1);
    while (*str1 == *str2 && *str1 && *str2) 
        str1 ++ ,str2 ++;
    return (*str1 - *str2) ;
}

unsigned char isnum (char ch) 
{
    char codeA[] = "0123456789abcdef";
    char codeB[] = "0123456789ABCDEF";
    int  i = 0 ;

    for ( i = 0 ; codeA[i] ;i  ++ ) 
        if ( codeA[i] == ch || codeB[i] == ch ) 
            return (i);
    return (-1);
}

long strtol (const char *src,char **err_pos,int base) 
{
    long ret = 0;
    unsigned char nr = 0;

    if (!base) 
        return (0);

    while ( *src ) {
        if ( (-1) != (nr = isnum (*src ++)) ) {
            ret = ret * base + nr;
        }
        else {
            if ( err_pos != NULL )
                (*err_pos) = (char*)src;
            return (ret);
        }
    }
    return ( ret ) ;
}

int strlen (const char *src) 
{
    int len = 0 ;
	while (*src ++) len ++;
	return (len) ;
}

char *itoa (unsigned long decem,char *buf,unsigned char base) 
{
	unsigned long  tmp = decem,index = 0;
	unsigned char  ch = 0 ;

	//split
	do {
		ch = tmp % base + 0x30;
		if (ch > 0x39) 
			ch += 0x07;
		buf[index ++]  = ch;
		tmp /= base ;
	} while (tmp != 0);
	buf[index] = '\0';//this must be here !!!
	//reserv
	for (tmp = 0,index --  ; tmp < index ; tmp ++ ,index --) {
		ch = buf[tmp] ;
		buf[tmp] = buf[index];
		buf[index] = ch;
	}
	return (buf);	
}

int sprintf (char* dbuf,const char* fmt,...)
{
    va_list va_p = (void*)0;
    int len = 0;

    va_start (va_p,fmt);
    len = vsprintf (dbuf,fmt,va_p);
    va_end (va_p);
    return ( len ) ;
}
/* we use this so that we can do without the ctype library */
#define is_digit(c)	((c) >= '0' && (c) <= '9')
static int skip_atoi(const char **s)
{
	int i=0;

	while (is_digit(**s))
		i = i*10 + *((*s)++) - '0';
	return i;
}

/* 
 * this is copied from linux 0.11
 * some bug fixed by casey 
 */

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define SMALL	64		/* use 'abcdef' instead of 'ABCDEF' */

static char * number(char * str, int num, int base, int size, int precision
	,int type)
{
	char c,sign,tmp[36];
	const char *digits="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	if (type&SMALL) digits="0123456789abcdefghijklmnopqrstuvwxyz";
	if (type&LEFT) type &= ~ZEROPAD;
	if (base<2 || base>36)
		return 0;
	c = (type & ZEROPAD) ? '0' : ' ' ;
	if (type&SIGN && num<0) {
		sign='-';
		num = -num;
	} else {
		sign=(type&PLUS) ? '+' : ((type&SPACE) ? ' ' : 0);
    }
	if (sign) size--;
	if (type&SPECIAL)
    {
		if (base==16) size -= 2;
		else if (base==8) size--;
        else;
    }
	i=0;
	if (num==0)
		tmp[i++]='0';
	else while (num!=0)
    {
		tmp[i++]=digits[ ((unsigned)num)%base ];
        num =(unsigned)num /  base;
    }
	if (i>precision) precision=i;
	size -= precision;
	if (!(type&(ZEROPAD+LEFT)))
		while(size-->0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;
	if (type&SPECIAL)
    {
		if (base==8)
			*str++ = '0';
		else if (base==16) {
			*str++ = '0';
			*str++ = digits[33];
		}
    }
	if (!(type&LEFT))
		while(size-->0)
			*str++ = c;
	while(i<precision--)
		*str++ = '0';
	while(i-->0)
		*str++ = tmp[i];
	while(size-->0)
		*str++ = ' ';
	return str;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
	int len;
	int i;
	char * str;
	char *s;
	int *ip;

	int flags;		    /* flags to number() */
	int field_width;	/* width of output field */
	int precision;		/* min. # of digits for integers; max
				         * number of chars for from string 
                         */
	int qualifier;		/* 'h', 'l', or 'L' for integer fields */

	for (str=buf ; *fmt ; ++fmt) {
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}
			
		/* process flags */
		flags = 0;
		repeat:
			++fmt;		/* this also skips first '%' */
			switch (*fmt) 
            {
				case '-': flags |= LEFT;    goto repeat;
				case '+': flags |= PLUS;    goto repeat;
				case ' ': flags |= SPACE;   goto repeat;
				case '#': flags |= SPECIAL; goto repeat;
				case '0': flags |= ZEROPAD; goto repeat;
            }
		
		/* get field width */
		field_width = -1;
		if (is_digit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}
		/* get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;	
			if (is_digit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}
		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
			qualifier = *fmt;
			++fmt;
		}
		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT))
				while (--field_width > 0)
					*str++ = ' ';
			*str++ = (unsigned char) va_arg(args, int);
			while (--field_width > 0)
				*str++ = ' ';
			break;
		case 's':
			s = va_arg(args, char *);
			len = strlen(s);
			if (precision < 0)
				precision = len;
			else if (len > precision)
				len = precision;
			if (!(flags & LEFT))
				while (len < field_width--)
					*str++ = ' ';
			for (i = 0; i < len; ++i)
				*str++ = *s++;
			while (len < field_width--)
				*str++ = ' ';
			break;
		case 'o':
			str = number(str, va_arg(args, unsigned long), 8,
				field_width, precision, flags);
			break;
		case 'p':
			if (field_width == -1) {
				field_width = 8;
				flags |= ZEROPAD;
			}
			str = number(str,
				(unsigned long) va_arg(args, void *), 16,
				field_width, precision, flags);
			break;
		case 'x':
			flags |= SMALL;
		case 'X':
			str = number(str, va_arg(args, unsigned long), 16,
				field_width, precision, flags);
			break;
		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			str = number(str, va_arg(args, unsigned long), 10,
				field_width, precision, flags);
			break;
		case 'n':
			ip = va_arg(args, int *);
			*ip = (str - buf);
			break;
		default:
			if (*fmt != '%')
				*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			break;
		}
	}
	*str = '\0';
	return (str-buf);
}

inline void * memcpy(void * dest,const void * src, unsigned long size)
{
    unsigned char *q = (unsigned char *)dest,*p = (unsigned char*)src;
    while (size --)
        *q ++ = *p ++;
    return (dest);
}

inline void * memset(void * s,char c,int count)
{
    unsigned char *p = (unsigned char*)s;

    while (count --) 
        *p ++ = c;
    return (s);
}

inline void *memmove (void *dest,void *src,int count)
{
    if (!dest || !src || !count) 
        return (void*)0;
    unsigned char *q = (unsigned char*)dest;
    unsigned char *p = (unsigned char*)src;

    if (q < p || q > p + count) 
        while (count --) *q ++ = *p ++;
    else {
        p += count;
        q += count;
        while (count --) *q -- = *p --;
    }
    return (dest);
}
