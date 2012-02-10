#ifndef _ASM_TCT_ELF_H
#define _ASM_TCT_ELF_H

#include <linux/ptrace.h>

/*
 * ELF register definitions 
 */

/* Processor specific flags for the ELF header e_flags field. */
#define EF_TCT_PIC		0x00000001
#define EF_TCT_FDPIC		0x00000002

#define ELF_FDPIC_CORE_EFLAGS EF_TCT_FDPIC

/*
 * ELF relocation types
 */
#define R_TCT_NONE		0
#define R_TCT_8			1
#define R_TCT_16		2
#define R_TCT_32		3
#define R_TCT_HI16		4
#define R_TCT_LO16		5
#define R_TCT_GPREL16		6
#define R_TCT_CALL		7
#define R_TCT_BRANCH		8
#define R_TCT_GNU_VTINHERIT	9
#define R_TCT_GNU_VTENTRY	10

/* ELF register definitions */
typedef unsigned long elf_greg_t;

#define ELF_NGREG (sizeof(struct pt_regs) / sizeof(elf_greg_t))
typedef elf_greg_t elf_gregset_t[ELF_NGREG];

/* TCT does not have fp regs */
typedef unsigned long elf_fpreg_t;
typedef elf_fpreg_t   elf_fpregset_t;

/*
 * This is used to ensure we don't load something for the wrong architecture.
 */
#define elf_check_arch(x) (((x)->e_machine == EM_TCT) || ((x)->e_machine == EM_TCT_OLD))
#define elf_check_fdpic(x) (1)
#define elf_check_const_displacement(x) (1)

/*
 * These are used to set parameters in the core dumps.
 */
#define ELF_CLASS	ELFCLASS32
#define ELF_DATA	ELFDATA2MSB
#define ELF_ARCH	EM_TCT
#define EM_TCT		0x8A
#define EM_TCT_OLD	0x666 // FIXME

#define ELF_PLAT_INIT(_r, load_addr)	do { } while(0)

#define ELF_FDPIC_PLAT_INIT(_regs, _exec_map_addr, _interp_map_addr, _dynamic_addr)	\
do { \
	_regs->r6 = 0; /* 6th argument to uclibc start: rtld_fini = NULL */ \
	_regs->r11	= _exec_map_addr;				\
	_regs->r12	= _interp_map_addr;				\
	_regs->r13	= _dynamic_addr;				\
} while(0)

#define USE_ELF_CORE_DUMP
#define ELF_EXEC_PAGESIZE	4096

/* This is the location that an ET_DYN program is loaded if exec'ed.  Typical
   use of this is to invoke "./ld.so someprog" to test out a new version of
   the loader.  We need to make sure that it is out of the way of the program
   that it will "exec", and that there is sufficient room for the brk.  */

// TODO: change this value as soon as we use ET_DYN
#define ELF_ET_DYN_BASE         0xD0000000UL

/* This yields a mask that user programs can use to figure out what
   instruction set this cpu supports.  */

#define ELF_HWCAP	(0)

/* This yields a string that ld.so will use to load implementation
   specific libraries for optimization.  This is more specific in
   intent than poking at uname or /proc/cpuinfo.  */

#define ELF_PLATFORM  (NULL)

// TODO SET_PERSONALITY
//#ifdef __KERNEL__
//#define SET_PERSONALITY(ex, ibcs2) set_personality((ibcs2)?PER_SVR4:PER_LINUX)
//#endif


#endif
