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
 * Based on
 *
 *  arch/v850/kernel/signal.c
 *  Copyright (C) 2001,02,03  NEC Electronics Corporation
 *  Copyright (C) 2001,02,03  Miles Bader <miles@gnu.org>
 *  Copyright (C) 1999,2000,2002  Niibe Yutaka & Kaz Kojima
 *  Copyright (C) 1991,1992  Linus Torvalds
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * 1997-11-28  Modified for POSIX.1b signals by Richard Henderson
 */

#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/ptrace.h>
#include <linux/unistd.h>
#include <linux/stddef.h>
#include <linux/personality.h>
#include <linux/tty.h>
#include <linux/hardirq.h>

#include <asm/uaccess.h>
#include <asm/ucontext.h>
#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/thread_info.h>
#include <asm/cacheflush.h>

#define DEBUG_SIG 0

#define _BLOCKABLE (~(sigmask(SIGKILL) | sigmask(SIGSTOP)))

asmlinkage int manage_signals(int retval, struct pt_regs* regs);

int do_signal(int retval, struct pt_regs *regs, int* handled);

asmlinkage int
sys_sigaltstack(const stack_t *uss, stack_t *uoss,
               struct pt_regs *regs)
{
}


struct sigframe
{
};

struct rt_sigframe {

};

static int restore_sigcontext(struct pt_regs *regs,
				struct sigcontext __user *sc, int *rval_p)
{
	unsigned int err = 0;
	return err;
}

asmlinkage int sys_rt_sigreturn(struct pt_regs *regs)
{
	return 0;
}

/*
 * Set up a signal frame.
 */
static int setup_sigcontext(struct sigcontext *sc, struct pt_regs *regs,
		 unsigned long mask)
{
	int err = 0;
	return err;
}

/*
 * Determine which stack to use..
 */
static inline void __user *get_sigframe(struct k_sigaction *ka,
		struct pt_regs *regs, size_t frame_size)
{
	return;
}

static int setup_frame(int sig, struct k_sigaction *ka,
			sigset_t *set, struct pt_regs *regs)
{
	
	return -1;
}

static int handle_signal(int retval, unsigned long sig, siginfo_t *info,
		struct k_sigaction *ka, sigset_t *oldset, struct pt_regs *regs)
{
	return -1;
}

/*
 * Note that 'init' is a special process: it doesn't get signals it doesn't
 * want to handle. Thus you cannot kill init even with a SIGKILL even by
 * mistake.
 *
 * Note that we go through the signals twice: once to check the signals that
 * the kernel can handle, and then we build all the user-level signal handling
 * stack-frames in one go after that.
 */
int do_signal(int retval, struct pt_regs *regs, int *handled)
{
return -1;
}

asmlinkage int manage_signals(int retval, struct pt_regs *regs)
{
		return -1;
}

asmlinkage void manage_signals_irq(struct pt_regs *regs)
{

}
