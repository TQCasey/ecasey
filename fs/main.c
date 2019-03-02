#include <stdio.h>
#include <string.h>

#include <ecasey/fs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ecasey/exec.h>
#include <2440/ports.h>

LOCAL i8 buf[NR_ARGC * NR_SRC_SIZE] = {0};
LOCAL i8 argv[NR_ARGC][NR_SRC_SIZE];
extern u32 getchs (i8 *buf,u32);
extern i32 getargv (RO i8* src,i8 argv[][NR_SRC_SIZE]);

#define LECHO(buf)      puts(buf)

extern i32 init_buffer (void);
extern void init_iscache (void);
extern void init_ftbl (void);
extern u32 sync_blks (u32 blks_cnt) ;
extern void load_rootfs (u16 dev_no);
extern void put_rootfs (u16 dev_no);
extern void warn (RO i8 *fmt,...);
extern void perr (i32 errno);
extern void sync_dev_inodes (void);
extern i32 sync_tl (void);
extern i32 create_ramdsk (void);

extern i32 do_mkdir (RO i8 *path ,u16 mode) ;
extern i32 do_rmdir (RO i8 *path) ;
extern i32 do_ls (RO i8 *path);
extern i32 do_chdir (RO i8 *path);
extern i32 do_chroot (RO i8 *path);
extern i32 do_chmod (RO i8 *path,u16 mode);
extern i32 do_mknod (RO i8* path,u16 mode,u16 dev);
extern i32 do_close (i32 pid,i32 fd);
extern i32 do_open (i32 pid,RO i8 *path,i32 flag,i32 mode);
extern i32 do_dumpnod (RO i8 *path);
extern i32 do_hlink (RO i8 *old_name,RO i8 *new_name) ;
extern i32 do_unhlink (RO i8* name) ;
extern i32 do_write (u32 pid,i32 fd,i8 * buf,i32 count);
extern i32 do_read (u32 pid,i32 fd,i8 * buf,i32 count);
extern i32 do_lseek (u32 pid,i32 fd,u32 offset,i32 origin);
extern i32 do_mount (RO i8 *dev_name,RO i8 *dir_name,u32 rw);
extern i32 do_umount (RO i8 *dir_name);
extern i32 do_fstat (i32 pid,i32 fd,struct stat *p) ;
extern i32 do_stat (RO i8 *fname,struct stat *p);
extern i32 do_dupfd (i32,i32);
extern i32 do_dup2 (i32,i32,i32);
extern struct proc_st *proc;
extern i32 elfload (i32 pid,i8 *bin,u32 size,RO i8 *path) ;

int main (void)
{
	u16 dev = DEV_NAND;
    i32 ret,len,i;

    printf ( "$ > total buffers %d \n",init_buffer () );
	init_iscache ();
    init_ftbl ();
    load_rootfs (dev);
    create_ramdsk ();

    while (true) {
        PROMOT ();
        memset (argv,0,sizeof(argv));
        memset (buf,0,sizeof(buf));
        getchs (buf,sizeof(buf));
        getargv (buf,(i8 (*) [NR_SRC_SIZE])argv);

        if (!strcmp (argv[0],"q")) {
            break;
		} else if (!strcmp (argv[0], "mkdir")) {
            perr (do_mkdir (argv[1],0));
        } else if (!strcmp (argv[0],"rmdir")) {
            perr (do_rmdir (argv[1]));
        } else if (!strcmp (argv[0],"ls")) {
            perr (do_ls (argv[1]));
        } else if (!strcmp (argv[0],"cd")) {
            perr (do_chdir (argv[1]));
        } else if (!strcmp (argv[0],"chroot")) {
            perr (do_chroot (argv[1]));
        } else if (!strcmp (argv[0],"chmod")) {
            perr (do_chmod (argv[1],0));
        } else if (!strcmp (argv[0],"mount")) {
            perr (do_mount (argv[1],argv[2],1));
        } else if (!strcmp (argv[0],"umount")) {
            perr (do_umount (argv[1]));
        } else if (!strcmp (argv[0],"mknod")) {
            u16 mod;
            if (!strcmp (argv[1],"-b"))
                mod = I_BLOCK_SPECIAL;
            else if (!strcmp (argv[1],"-c")) 
                mod = I_CHAR_SPECIAL;
            else {
                printf ( "wrong node type !\n" );
                continue;
            }
            perr (do_mknod (argv[2],mod,atoi (argv[3])));
        } else if (!strcmp (argv[0],"ln")) {
            perr (do_hlink (argv[1],argv[2]));
        } else if (!strcmp (argv[0],"rm")) {
            perr (do_unhlink (argv[1]));
        } else if (!strcmp (argv[0],"dpnod")) {
            perr (do_dumpnod (argv[1]));
        } else if (!strcmp (argv[0],"cat")) {
            i32 fd = do_open (proc->pid,argv[1],O_RDONLY,0);
            if (fd < 0) {
                printf ( "open file failed !\n" );
                continue;
            }
            while ((ret = do_read (proc->pid,fd,buf,sizeof(buf)))) 
                for (i = 0; i < ret ;i++) 
                    printf ( "%c",buf[i]);
            do_close (proc->pid,fd);
        } else if (!strcmp (argv[0],"cp")) {
            i32 fdrd = do_open (proc->pid,argv[1],O_RDONLY,0);
            if (fdrd < 0) {
                printf ( "open file failed !\n" );
                continue;
            }
            i32 fd = do_open (proc->pid,argv[2],O_CREAT | O_RDWR,0);
            if (fd < 0) {
                printf ( "open file failed !\n" );
                do_close (proc->pid,fdrd);
                continue;
            }
            while ((len = do_read (proc->pid,fdrd,buf,sizeof(buf)))) 
                do_write (proc->pid,fd,buf,len);
            do_close (proc->pid,fd);
            do_close (proc->pid,fdrd);
        } else if (!strcmp (argv[0],"mv")) {
            i32 fdrd = do_open (proc->pid,argv[1],O_RDONLY,0);
            if (fdrd < 0) {
                printf ( "open file failed !\n" );
                continue;
            }
            i32 fd = do_open (proc->pid,argv[2],O_CREAT | O_RDWR,0);
            if (fd < 0) {
                printf ( "open file failed !\n" );
                do_close (proc->pid,fdrd);
                continue;
            }
            while ((len = do_read (proc->pid,fdrd,buf,sizeof(buf)))) 
                do_write (proc->pid,fd,buf,len);
            do_close (proc->pid,fd);
            do_close (proc->pid,fdrd);
            do_unhlink (argv[1]);
        } else if (!strcmp (argv[0],"load")) {
            perr (elfload (proc->pid,buf,sizeof(buf),argv[1]));
        } else if (!strcmp (argv[0],"write")) {
            i32 fd = do_open (proc->pid,argv[1],O_CREAT | O_RDWR,0);
            if (fd < 0) {
                printf ( "open file failed !\n" );
                continue;
            }
            do_write (proc->pid,fd,buf,sizeof(buf));
            do_close (proc->pid,fd);
		} else { 
            printf ( "command not found !\n" );
        }
    }
    put_rootfs (dev);
    sync_dev_inodes ();
    sync_blks (-1);
    sync_tl ();
    printf ( "sync done !\n" );
    while (true);
}
