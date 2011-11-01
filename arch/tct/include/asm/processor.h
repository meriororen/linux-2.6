#ifndef _ASM_TCT_PROCESSOR_H
#define _ASM_TCT_PROCESSOR_H

/*
 * Default implementation of macro that returns current
 * instruction pointer ("program counter").
 */
#define current_text_addr() ({ __label__ _l; _l: &&_l;})

#include <linux/compiler.h>
#include <linux/threads.h>
#include <asm/types.h>
#include <asm/ptrace.h>
#include <asm/current.h>

/*
 * User space process size: 3.75GB. Hardcoded into a few places.
 */
#define TASK_SIZE	(0xF0000000)

/*
 * Decides where the kernel will search for a free chunk of vm
 * during mmap's. No mmu means no vm means we won't need it.
 */
#define TASK_UNMAPPED_BASE	0

/*
 * If you change this structure, you must change the code and offsets
 * in asm-offsets.c
 */

struct thread_struct {
	unsigned long ksp;	/* kernel stack pointer */
	unsigned long usp; 	/* user stack pointer */
	unsigned long which_stack; /* 0 : kernel stack, 1 : user stack */
	void *	      debuggerinfo;
}

#define KSTK_TOS(tsk) ((unsigned long)task_stack_page(tsk) + THREAD_SIZE - 32)
#define task_pt_regs(tsk) ((struct pt_regs *)KSTK_TOS(tsk) - 1)
#define KSTK_EIP(tsk) 0
#define KSTK_ESP(tsk) 0
