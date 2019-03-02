#ifndef __EXEC_H__
#define __EXEC_H__

#define EI_NIDENT		(16)

/* elf type */
typedef unsigned int	elf32_addr;  
typedef unsigned short	elf32_half; 
typedef unsigned int	elf32_off;   
typedef signed int		elf32_sword;  
typedef unsigned int	elf32_word;  

/* elf header */
struct elf32_ehdr{      
	unsigned char e_ident[EI_NIDENT];      
	elf32_half  e_type;      
	elf32_half  e_machine;      
	elf32_word  e_version;      
	elf32_addr  e_entry;      
	elf32_off   e_phoff;      
	elf32_off   e_shoff;      
	elf32_word  e_flags;      
	elf32_half  e_ehsize;      
	elf32_half  e_phentsize;     
	elf32_half  e_phnum;      
	elf32_half  e_shentsize;     
	elf32_half  e_shnum;      
	elf32_half  e_shstrndx;  
}; 

/* program header */
struct elf32_phdr{      
	elf32_word  p_type;      
	elf32_off   p_offset;      
	elf32_addr  p_vaddr;      
	elf32_addr  p_paddr;      
	elf32_word  p_filesz;      
	elf32_word  p_memsz;      
	elf32_word  p_flags;      
	elf32_word  p_align;  
}; 

#define NR_ARGC         (32)
#define NR_SRC_SIZE     (64)

#define PROMOT()        printf ("# ")

#endif
