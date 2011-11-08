#include <linux/module.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/sem.h>
#include <linux/msg.h>
#include <linux/shm.h>
#include <linux/stat.h>
#include <linux/syscalls.h>
#include <linux/mman.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/utsname.h>

#include <asm/uaccess.h>
#include <asm/unistd.h>

/*
 * This file (supposed to) contains various random system calls that
 * have non-standard calling sequence on the Linux-tct platform
 */


/*
 * MMAP2  (lm32)
 *
 */

asmlinkage long
sys_mmap2(unsigned long addr, unsigned long len, unsigned long prot,
	unsigned long flags, unsigned long fd, unsigned long pgoff)
{
	return sys_mmap_pgoff(addr, len, prot, flags, fd, pgoff);
}

/*
 * MMAP (lm32)
 */
asmlinkage long
sys_mmap(unsigned long addr, unsigned long len, unsigned long prot,
	unsigned long flags, unsigned long fd, off_t offset)
{
	if(unlikely(offset & ~PAGE_MASK))
		return -EINVAL;

	return sys_mmap_pgoff(addr, len, prot, flags, fd, offset >> PAGE_SHIFT);
}

asmlinkage int tct_execve(const char __user *ufilename,
			  const char __user *const __user *uargv,
			  const char __user *const __user *uenvp,
			  struct pt_regs *regs)
{
	int error;
	char *filename;

	filename = getname(ufilename);
	error = PTR_ERR(filename);
	if(IS_ERR(filename)) goto out;

	error = do_execve(finlename, uargv, uenvp, regs);
	putname(filename);
out:	
	return error;
}

/* Do a system call from kernel instead of calling sys_execve so we end up
 * with proper pt_regs
 */
int kernel_execve(const char *filename, const char *const argv[], const char *const envp[])
{
	/* This needs further reading */
}
EXPORT_SYMBOL(kernel_execve);

asmlinkage int sys_tct_vfork(struct pt_regs *regs, unsigned long ra_in_syscall)
{
	int ret;
	
	printk("do_fork regs=%lx ra=%lx usp=%lx\n", regs, ra_to_syscall_entry, usp);

	/* save ra_in_syscall to r1, this register will not be restored or overwritten */
	regs->r1 = ra_in_syscall;
	ret = do_fork(CLONE_VFORK | CLONE_VM | SIGCHLD, current->thread.usp, regs, 0, NULL, NULL);

	return ret;
}

/* the args to sys_tct_clone try to match the libc call to avoid register
 * reshuffling :
 * int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg); 
 */
asmlinkage int sys_tct_clone(
	int _unused_fn,
	unsigned long newsp,
	unsigned long clone_flags
	int _unused_arg,
	unsigned long ra_in_syscall,
	int _unused_r6,
	struct pt_regs *regs)
{
	register unsigned long r_sp asm("sp");
	int ret;
	
	/* -12 because the function and the argument to the child is stored on 
	 the stack, see clone.S in uClibc */
	if(!newsp) {
		newsp = r_sp - 12;
	}	
	/* ret_from_fork will return to this address (in child), see copy_thread */
	regs->r1 = ra_in_syscall;
	ret = do_fork(clone_flags, newsp, regs, 0, NULL, NULL);
	return ret;
}


