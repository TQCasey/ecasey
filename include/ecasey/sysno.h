#ifndef __SYSNO__
#define __SYSNO__


/* sys calls message */
#define SYS_GET_TICK    0x00000000
#define SYS_UART_WRITE  0x00000001
#define SYS_LCD_WRITE   0x00000002
#define SYS_PAUSE       0x00000003
#define SYS_GETTIME     0x00000004
#define SYS_SETTIME     0x00000005
#define SYS_GETADC      0x00000006
#define SYS_GETPOS      0x00000007
#define SYS_RW_NAND     0x00000008
#define SYS_GETCHS      0x00000009
#define SYS_SYNC_TL     0x0000000A

#define MAX_SYSCALL     (256)

#endif
