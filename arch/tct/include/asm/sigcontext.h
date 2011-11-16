/*
 * From Theobroma Systems' UCLINUX
 */

#ifndef	_ASM_TCT_SIGCONTEXT_H
#define _ASM_TCT_SIGCONTEXT_H

#include <asm/ptrace.h>

/*
 * Signal context structure - contains all info to do with the state before the 
 * signal handler was invoked.  Note: only add new entries to the end of the 
 * structure.
 */
struct sigcontext {
	struct pt_regs regs;
	unsigned long oldmask;
};

#endif
