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

#include <linux/ptrace.h>
#include <asm/uaccess.h>

void ptrace_disable(struct task_struct *child)
{
	/* nothing todo - we have no single step */
}

static int ptrace_getregs(struct task_struct *child, unsigned long __user *data)
{
	return -1;
}

static int ptrace_setregs (struct task_struct *child, unsigned long __user *data)
{
	return -1;
}

long arch_ptrace(struct task_struct *child, long request, unsigned long addr,
	unsigned long data)
{
	return 0;
}

