/*
 * HW exception handling
 *
 */

#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <linux/module.h>

#include <asm/exceptions.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/ptrace.h>
#include <asm/current.h>

#define TCT_PRIVILIGE 	0x1
#define TCT_UNDEFINED 	0x2
#define TCT_ITLB      	0x3
#define TCT_DTLB      	0x4
#define TCT_FETCH_ABORT	0x5
#define TCT_DATA_ABORT	0x6
#define TCT_ZERO_DIV	0x7

void die(const char *str, struct pt_regs *fp, long err)
{
	console_verbose();
	spin_lock_irq(&die_lock);
	printk(KERN_WARNING "Oops: %s, sig: %ld\n", str, err);
	show_regs(fp);
	spin_unlock_irq(&die_lock);
	
	do_exit(err);
}

void _exception(int signr, struct pt_regs *regs, int code, unsigned long addr)
{
	siginfo_t info;

	if (kernel_mode(regs)) {
		die("Exception in kernel mode", regs, signr);
	}
	info.si_signo = signr;
	info.si_errno = 0;
	info.si_code = code;
	info.si_addr = (void __user *)addr;
	force_sig_info(signr, &info, current);
}

asmlinkage void full_exception(struct pt_regs *regs, unsigned int type,
							int fsr, int addr)
{


#if 1
	printk(KERN_WARNING "Exception %02x in %s mode, FSR=%08x PC=%08x " \
							"ESR=%08x\n",
			type, user_mode(regs) ? "user" : "kernel", fsr,
			(unsigned int) regs->pc, (unsigned int) regs->esr);
#endif

	switch (type & 0x1F) {
	case MICROBLAZE_ILL_OPCODE_EXCEPTION:
		if (user_mode(regs)) {
			pr_debug("Illegal opcode exception in user mode\n");
			_exception(SIGILL, regs, ILL_ILLOPC, addr);
			return;
		}
		printk(KERN_WARNING "Illegal opcode exception " \
							"in kernel mode.\n");
		die("opcode exception", regs, SIGBUS);
		break;
	case MICROBLAZE_IBUS_EXCEPTION:
		if (user_mode(regs)) {
			pr_debug("Instruction bus error exception in user mode\n");
			_exception(SIGBUS, regs, BUS_ADRERR, addr);
			return;
		}
		printk(KERN_WARNING "Instruction bus error exception " \
							"in kernel mode.\n");
		die("bus exception", regs, SIGBUS);
		break;
	case MICROBLAZE_DBUS_EXCEPTION:
		if (user_mode(regs)) {
			pr_debug("Data bus error exception in user mode\n");
			_exception(SIGBUS, regs, BUS_ADRERR, addr);
			return;
		}
		printk(KERN_WARNING "Data bus error exception " \
							"in kernel mode.\n");
		die("bus exception", regs, SIGBUS);
		break;
	case MICROBLAZE_DIV_ZERO_EXCEPTION:
		if (user_mode(regs)) {
			pr_debug("Divide by zero exception in user mode\n");
			_exception(SIGILL, regs, FPE_INTDIV, addr);
			return;
		}
		printk(KERN_WARNING "Divide by zero exception " \
							"in kernel mode.\n");
		die("Divide by zero exception", regs, SIGBUS);
		break;
	case MICROBLAZE_FPU_EXCEPTION:
		pr_debug("FPU exception\n");
		/* IEEE FP exception */
		/* I removed fsr variable and use code var for storing fsr */
		if (fsr & FSR_IO)
			fsr = FPE_FLTINV;
		else if (fsr & FSR_OF)
			fsr = FPE_FLTOVF;
		else if (fsr & FSR_UF)
			fsr = FPE_FLTUND;
		else if (fsr & FSR_DZ)
			fsr = FPE_FLTDIV;
		else if (fsr & FSR_DO)
			fsr = FPE_FLTRES;
		_exception(SIGFPE, regs, fsr, addr);
		break;

#ifdef CONFIG_MMU
	case MICROBLAZE_PRIVILEGED_EXCEPTION:
		pr_debug("Privileged exception\n");
		_exception(SIGILL, regs, ILL_PRVOPC, addr);
		break;
#endif
	default:
	/* FIXME what to do in unexpected exception */
		printk(KERN_WARNING "Unexpected exception %02x "
			"PC=%08x in %s mode\n", type, (unsigned int) addr,
			kernel_mode(regs) ? "kernel" : "user");
	}
	return;
}
