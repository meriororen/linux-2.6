/*
 * (C) Copyright 2007
 *     Theobroma Systems <www.theobroma-systems.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * This file handles the architecture-dependent parts of process handling..
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/stddef.h>
#include <linux/unistd.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/user.h>
#include <linux/interrupt.h>
#include <linux/reboot.h>
#include <linux/fs.h>
#include <linux/io.h>


#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/traps.h>
#include <asm/setup.h>
#include <asm/pgtable.h>

#include <asm/hw/tct-board.h>

asmlinkage void ret_from_fork(void);

struct thread_info* tct_current_thread;
/*
 * The following aren't currently used.
 */
void (*pm_idle)(void);
EXPORT_SYMBOL(pm_idle);

void (*pm_power_off)(void);
EXPORT_SYMBOL(pm_power_off);


/*
 * The idle loop on TCT
 */
static void default_idle(void)
{
	while(!need_resched())
		__asm__ __volatile__("and r0, r0, r0" :::"memory");
}

void (*idle)(void) = default_idle;

/*
 * The idle thread. There's no useful work to be
 * done, so just try to conserve power and have a
 * low exit latency (ie sit in a loop waiting for
 * somebody to say that they'd like to reschedule)
 */
void cpu_idle(void)
{
	/* endless idle loop with no priority at all */
	while (1) {
		idle(); //default idle
		preempt_enable_no_resched();
		schedule();
		preempt_disable();
	}
}

void machine_restart(char *__unused)
{
	//how does TCT restart a machine?
}

void machine_halt(void)
{
	printk("%s:%d: machine_halt() is not possible on tct\n", __FILE__, __LINE__);
	for(;;)
		cpu_relax();
}

void machine_power_off(void)
{
	printk("%s:%d: machine_power_off() is not possible on tct\n", __FILE__, __LINE__);
	for(;;)
		cpu_relax();
}

void show_regs(struct pt_regs * regs)
{
	printk("Registers:\n");
	#define TCTREG(name) printk("%3s : 0x%lx\n", #name, regs->name)
	TCTREG(r0);  TCTREG(r1);  TCTREG(r2);  TCTREG(r3);  TCTREG(r4);
	TCTREG(r5);  TCTREG(r6);  TCTREG(r7);  TCTREG(r8);  TCTREG(r9);
	TCTREG(r10); TCTREG(r11); TCTREG(r12); TCTREG(r13); TCTREG(r14);
	TCTREG(r15); TCTREG(r16); TCTREG(r17); TCTREG(r18); TCTREG(r19);
	TCTREG(r20); TCTREG(r21); TCTREG(r22); TCTREG(r23); TCTREG(r24);
	TCTREG(r25); TCTREG(lkr);  TCTREG(fp);  TCTREG(sp);  TCTREG(bos);
	TCTREG(brp1);  TCTREG(brp2);
	#undef TCTREG
}

static void kernel_thread_helper(int reserved, int (*fn)(void*), void * arg)
{
	do_exit(fn(arg));
}

/*
 * Create a kernel thread
 */
int kernel_thread(int (*fn)(void*), void * arg, unsigned long flags)
{
	/* prepare registers from which a child task switch frame will be copied */
	struct pt_regs regs;

	set_fs(KERNEL_DS);

	memset(&regs, 0, sizeof(regs));

	//printk ("kernel_thread fn=%x arg=%x regs=%x\n", fn, arg, &regs);

	regs.r2 = (unsigned long)fn;
	regs.r3 = (unsigned long)arg;
	regs.r5 = (unsigned long)kernel_thread_helper;
	regs.pt_mode = PT_MODE_KERNEL;

	return do_fork(flags | CLONE_VM | CLONE_UNTRACED, 0, &regs, 0, NULL, NULL);
}


void flush_thread(void)
{
	set_fs(USER_DS);
}

/* no stack unwinding */
unsigned long get_wchan(struct task_struct *p)
{
	return 0;
}

unsigned long thread_saved_pc(struct task_struct *tsk)
{
	return 0;
}

int copy_thread(unsigned long clone_flags,
		unsigned long usp, unsigned long stk_size,
		struct task_struct *p, struct pt_regs *regs)
{
	unsigned long child_tos = KSTK_TOS(p);
	struct pt_regs *childregs;

	if(!user_mode(regs)) {
		/* kernel thread */
		
		if(usp != 0)
			panic("trying to start kernel thread with usp != 0");

		/* childregs = full task switch frame on kernel stack of child */
		childregs = (struct pt_regs *)(child_tos) - 1;
		*childregs = *regs;

		childregs->r4 = 0;
		regs->r4 = p->pid;

		/* return via ret_from_fork */
		childregs->lkr = (unsigned long)ret_from_fork;

		/* setup ksp/usp */
		p->thread.ksp = (unsigned long)childregs - 4; /* perhaps not necessary */
		childregs->sp = p->thread.ksp;
		p->thread.usp = 0;
		p->thread.which_stack = 0; /* kernel stack */

		//printk("copy_thread1: ->pid=%d tsp=%lx r5=%lx p->thread.usp=%lx\n",
		//		p->pid, task_stack_page(p), childregs->r5, p->thread.ksp, p->thread.usp);
	} else {
		/* userspace thread (vfork, clone) */
		
		unsigned long ra_in_syscall;
		struct pt_regs* childsyscallregs;
	
		//asm volatile("break");

		/* this was brought to us by sys_tct_vfork in kernel/sys_tct.c */
		ra_in_syscall = regs->r1;

		/* childsyscallregs = full syscall frame on kernel stack of child */
		childsyscallregs = (struct pt_regs *)(child_tos) - 1;
		/* no need to return value here, it will be set by task switch frame */ 
		/* copy task switch frame, child shall return with the same register as parent entered the syscall except for return value of syscall */
		childregs = childsyscallregs - 1;
		
		/* child shall have same syscall context to restore as parent has */
		*childsyscallregs = *regs;

		regs->r4 = p->pid; /* parents get child's pid as the retval */
		
		/* user stack pointer is sharef with the parent per definition of vfork */
		p->thread.usp = usp;
		
		/* kernel stack pointer is not shared with parent, it is the beginning of the just created new task switch segment on the kernel itself */
		p->thread.ksp = (unsigned long)childregs - 4;
		p->thread.which_stack = 0; /* resume from ksp */

		/* child returns via ret_from_fork */
		childregs->lkr = (unsigned long)ret_from_fork;
		/* child shall return to where sys_vfork_wrapper has been called */
		childregs->r5 = ra_in_syscall;
		/* child gets zero as return value from syscall */
		childregs->r4 = 0;
		/* after task switch segment return the stack pointer shall point to the syscall frame */
		childregs->sp = (unsigned long)childsyscallregs - 4;
	
	}
	return 0;
}
