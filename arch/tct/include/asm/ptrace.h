#ifndef	_ASM_TCT_PTRACE_H
#define _ASM_TCT_PTRACE_H

#define PT_MODE_KERNEL 	1
#define PT_MODE_USER	0

#ifndef __ASSEMBLY__

typedef unsigned long tct_reg_t;

/* defines the way the registers are stored on the
   stack during a system call */
struct pt_regs {
	tct_reg_t r0;
	tct_reg_t r1;
	tct_reg_t r2;
	tct_reg_t r3;
	tct_reg_t r4;
	tct_reg_t r5;
	tct_reg_t r6;
	tct_reg_t r7;
	tct_reg_t r8;
	tct_reg_t r9;
	tct_reg_t r10;
	tct_reg_t r11;
	tct_reg_t r12;
	tct_reg_t r13;
	tct_reg_t r14;
	tct_reg_t r15;
	tct_reg_t r16;
	tct_reg_t r17;
	tct_reg_t r18;
	tct_reg_t r19;
	tct_reg_t r20;
	tct_reg_t r21;
	tct_reg_t r22;
	tct_reg_t r23;
	tct_reg_t r24;
	tct_reg_t r25;
	tct_reg_t lnk;
	tct_reg_t fp;
	tct_reg_t sp;
	tct_reg_t bos;
	tct_reg_t brp1;
	tct_reg_t brp2;
	unsigned int pt_mode;
}

#ifdef __KERNEL__
#define user_mode(regs) ((regs)->pt_mode == PT_MODE_USER)

#define instruction_pointer(regs) ((regs)->ea) /* points to EA (LM32) */
#define profile_pc(regs) instruction_pointer(regs)

//void show_regs(struct pt_regs *);

#else /* !__KERNEL__ */

/* TBD (gdbserver/ptrace */

#endif /* !__KERNEL__ */

/* Arbitrarily choose the same ptrace numbers as used by the Sparc code. */
#define PTRACE_GETREGS		12
#define PTRACE_SETREGS		13

#define PTRACE_GETFDPIC		31

#define PTRACE_GETFDPIC_EXEC	0
#define PTRACE_GETFDPIC_INTERP	1

/* for gdb */
#define PT_TEXT_ADDR	50
#define PT_TEXT_END_ADDR	51
#define PT_DATA_ADDR	52

#endif /* !__ASSEMBLY__ */

#endif /* _ASM_TCT_PTRACE_H */
