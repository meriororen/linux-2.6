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

/*
 * These are used to set parameters in the core dumps.
 */
#define ELF_CLASS	ELFCLASS32
#define ELF_DATA	ELFDATA2MSB
