#include <stdio.h>
#include <string.h>

#include <ecasey/fs.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ecasey/exec.h>
#include <sys/types.h>
#include <errno.h>

#define __DEBUG_ELF__

extern i32 do_open (i32 pid,RO i8 *path,i32 flag,i32 mode);
extern i32 do_close (i32 pid,i32 fd);
extern i32 do_read (u32 pid,i32 fd,i8 * buf,i32 count);

i32 elfload (i32 pid,i8 *bin,u32 size,const i8 *path) 
{
    i8 tmp [1024];
    struct elf32_ehdr *pe ;
    struct elf32_phdr *ph ;
    i32 fd ,len;
	RO i8 *p;

    if (!path || !bin || !size) 
        return (-ENOPATH);
	fd = do_open (pid,path,O_RDONLY,0);
	if (fd < 0) 
        return (-EOPNFAILD);
    /* read elf header fisrt */
    if (!(len = do_read (pid,fd,tmp,sizeof(tmp)))) {
		do_close (pid,fd);
		return (-EBADRD);
    }
    pe = (struct elf32_ehdr *)tmp;
    if ((pe->e_ident [0] != 0x7F) || strncmp ("ELF",(i8*)&(pe->e_ident [1]),3)) {
		do_close (pid,fd);
		return (-ENOTELF);
    }
#ifdef __DEBUG_ELF__
    /* ident data */
    printf ( "==================================\n" );
    printf ( "magic = %c%c%c \n",pe->e_ident [1],pe->e_ident [2],pe->e_ident [3] );
    if (!pe->e_ident [4]) {
		do_close (pid,fd);
		return (-EOINVALID);
    }
    printf ( "class = %d bits object \n",(pe->e_ident [4] == 1) ? 32:64 );
    if (!pe->e_ident [5]) {
		do_close (pid,fd);
		return (-EOINVALID);
    }
    printf ( "endia = %s \n",(pe->e_ident [5] != 1) ? "big":"small" );
    printf ( "ver   = %d.0\n",pe->e_ident [6] );

    /* type */
    switch (pe->e_type)
    {
        case 1      : p = "relocational"  ; break;
        case 2      : p = "executable"    ; break;
        case 3      : p = "dynamic lib"   ; break;
        case 4      : p = "core"          ; break;
        case 0xFF00 : p = "low proc"      ; break;
        case 0xFFFF : p = "high proc"     ; break;
        default     : 
			do_close (pid,fd);
			return (-EOINVALID);
    }
    printf ( "type  = %s file \n",p );
    /* machine */
    switch (pe->e_machine)
    {
        case 1      : p = "AT&T WE32100"    ; break;
        case 2      : p = "SPARC"           ; break;
        case 3      : p = "Intel"           ; break;
        case 4      : p = "Moto68K"         ; break;
        case 5      : p = "Moto88K"         ; break;
        case 7      : p = "Intel80860"      ; break;
        case 8      : p = "MIPS RS3000"     ; break;
        case 10     : p = "MIPS RS4000"     ; break;
        case 0x28   : p = "ARM"             ; break;
        default     : 
			do_close (pid,fd);
			return (-EOUNDEF);
    }
    printf ( "mach  = %s \n",p );
    /* entry */
    printf ( "entry = 0x%0x \n",pe->e_entry );
    /* program head offset */
    printf ( "phoff = 0x%0x \n",pe->e_phoff );
    /* section offset */
    printf ( "scoff = 0x%0x \n",pe->e_shoff );
    /* elf header size */
    printf ( "esize = 0x%0xB \n",pe->e_ehsize );
    /* program header size */
    printf ( "psize = 0x%0xB \n",pe->e_phentsize );
    /* section header size */
    printf ( "ssize = 0x%0xB \n",pe->e_shentsize );
    /* program header numbers */
    printf ( "ph_nr = 0x%x \n",pe->e_phnum );
    /* section index */
    printf ( "scidx = 0x%0x \n",pe->e_shstrndx );

    /* progarm header info here  */
    printf ( "==================================\n" );
    ph = (struct elf32_phdr *)(tmp + pe->e_phoff);

    switch (ph->p_type) 
    {
        case 0  : p = "unused"      ; break;
        case 1  : p = "loadable"    ; break;
        case 2  : p = "dynamic"     ; break;
        case 3  : p = "interp"      ; break;
        case 4  : p = "note"        ; break;
        default : p = "reserved"    ; break;
    }
    printf ( "ptype = %s \n",p );
    /* seg offset */
    printf ( "sgoff = 0x%0x \n",ph->p_offset );
    /* seg vaddr */
    printf ( "sgvad = 0x%0x \n",ph->p_vaddr );
    /* seg paddr */
    printf ( "sgpad = 0x%0x \n",ph->p_paddr );
    /* seg file size */
    printf ( "sgfsz = 0x%0x \n",ph->p_filesz );
    /* seg mem size */
    printf ( "sgmsz = 0x%0x \n",ph->p_memsz );
    /* seg align */
    printf ( "sgalg = 0x%0x \n",(elf32_word)ph->p_align );
    /* seg perm */
    switch (ph->p_flags)
    {
        case    1:
        case    4:
        case    5:  p = "r-x"       ; break;
        case    2:
        case    3:
        case    6:
        case    7:  p = "rwx"       ; break;
        default  :  p = "undefined" ; break;
    }
    printf ( "perm  = %s \n",p );
#else 
    if (pe->e_ident [4] != 1) {
		do_close (pid,fd);
		return (-EONOSURPOT);
    }
    if ((pe->e_type != 2) && (pe->e_type != 3)) {  /* binlib or exefileis surpoted ONLY */
		do_close (pid,fd);
		return (-EONOSURPOT);
    }
    if ((pe->e_machine != 0x28) && (pe->e_machine != 3)) {  /* intel or arm ONLY */
		do_close (pid,fd);
		return (-EONOSURPOT);
    }
#endif
    do_close (pid,fd);
    return (0);
}
