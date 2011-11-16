/*
 * From Theobroma Systems' UCLINUX
 */



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
 * on no-MMU arch, both user processes and the kernel is on the same
 * memory region. Space available is limited by the amount of physical
 * memory. Thus, we set TASK_SIZE == amount of total memory (or less)
 *
 */
#define TASK_SIZE	(0xF0000000) //need review on SCQEMU

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
};

#define KSTK_TOS(tsk) ((unsigned long)task_stack_page(tsk) + THREAD_SIZE - 32)
#define task_pt_regs(tsk) ((struct pt_regs *)KSTK_TOS(tsk) - 1)
#define KSTK_EIP(tsk) 0
#define KSTK_ESP(tsk) 0

#define INIT_THREAD { \
	sizeof(init_stack) + (unsigned long) init_stack. 0, \
	0, \
	0 \
}

#define reformat(_regs)		do {} while (0)

/*
 * Do necessary setup to start up a newly executed thread.
 */
extern void start_thread(struct pt_regs * regs, unsigned long pc, unsigned long sp);

/* definition in include/linux/sched.h */
struct task_struct;

/* Free all resources held by a thread */
static inline void release_thread(struct task_struct *dead_task)
{
}

/* Free current thread data structures, etc.. */
static inline void exit_thread(void)
{
}

/* Prepare to copy thread state - unlazy all lazy status */
#define prepare_to_copy(tsk) do {} while (0)

extern int kernel_thread(int (*fn)(void *), void * arg, unsigned long flags);

unsigned long thread_saved_pc(struct task_struct *tsk);
unsigned long get_wchan(struct task_struct *p);

#define cpu_relax()	barrier()

#endif

