#include <linux/sys.h>
#include <linux/linkage.h>
#include <asm/traps.h>
#include <asm/unistd.h>
#include <asm/thread_info.h>
#include <asm/errno.h>
#include <asm/setup.h>
#include <asm/segment.h>
#include <generated/asm-offsets.h>
#include <asm/ptrace.h>
#include <asm/hw/registers.h>


/* exception vector for os-aware gdb and kernel signals */
#define KERNEL_EXCEPTION_VECTOR(offset) \
	addi sp, sp, -132; \
	stm lkr, sp, 120; \
	call _save_syscall_frame; \
	mov r1, offset; \
	addi r2, sp, 4;
	call asm_do_sig;
	jmp _return_from_exception; \
	nop

.section ".exception.text", "ax"

ENTRY(reset_handler)
	KERNEL_EXCEPTION_VECTOR(0x0)
ENTRY(scall_handler)
	KERNEL_EXCEPTION_VECTOR(0x4)
ENTRY(cpu_handler)
	KERNEL_EXCEPTION_VECTOR(0x8)
ENTRY(mmu_handler)
	KERNEL_EXCEPTION_VECTOR(0xC)
ENTRY(interrupt_handler)
	KERNEL_EXCEPTION_VECTOR(0x10)



ENTRY(system_call)
	/* break */
	/* store away r9, r10 so that we can use it here */
	stm r9, sp, 0	/* needed for various */
	stm r10, sp, -4	/* needed for current = current_thread_info()->task */
	stm r11, sp, -8	/* needed for user stack pointer, if switching */
	
	/* test if already on kernel stack: test current_thread_info->task->which_stack */
	movh r9, hi(tct_current_thread)
	orr r9, lo(tct_current_thread)
	ldm r9, r9, 0 	/* dereference tct_current_thread */
	ldm r10, r9, TI_TASK /* load pointer to task */
	ldm r9, r10, TASK_WHICH_STACK /* reference to type of stack */

	mov r11, sp /* remember sp for restoring r9, r10, r11 */

	/* r9 now contains current task's which_stack */
	neq r0, r9, 0
	beq 1f

	/* we are on user stack, need to switch */
	stm sp, r10, TASK_USP /* store usp */
	ldm sp, r10, TASK_KSP /* load ksp */
	mov r0, 0
	stm r0, r10, TASK_WHICH_STACK 

1: /* already on kernel stack */
	
	addi sp, sp, -132
	stm r11, sp, 116
	
	/* restore r9, r10, r11 */
	ldm r9, r11, 0
	ldm r10, r11, -4
	ldm r11, r11, -8

   /* save registers */
	stm lkr, sp, 120
	call _save_syscall_frame
	
	/* r7 always holds the pointer to struct pt_regs */
	addi r7, sp, 4
	#addi r4, sp, 4
	
	/* r8 always holds the syscall number */
	/* check if syscall number is valid */
	mov r9, __NR_syscalls
	clt r0, r8, r9, 0 		/* r8 < r9 ? r0 = 1 : r0 = 0 */
	beq .badsyscall
	movh r9, hi(sys_call_table) /* load address of syscall table */
	orr r9, lo(sys_call_table)
	shl r10, r8, 2 
	add r9, r9, r10 /* add offset of syscall number to address */
	ldm r9, r9, 0 /* fetch address of syscall function */
	mov r0, r9 /* move it to r0  */
	call_reg /* execute syscall */

.syscallTail:
	/* store pt_regs* in r2 */
	addi r2, sp, 4
	call manage_signals
	stm r1, sp, 8 /* store return value into pt_regs */

	jmp _restore_and_return_exception

.badsyscall:
	mov r1, -ENOSYS
	
	jmp _restore_and_return_exception

/* end of exception handlers */

/********************************/
/* ensure to be on kernel stack */
/********************************/
#define ENSURE_TO_BE_ON_KERNEL_STACK \
	/* store away r9, r10 so that we can use it here */ \
	stm r9, sp, -4; \
	stm r10, sp, -8; \
	stm r11, sp, -12; \
	/* test if already on kernel stack: test current_thread_info->task->which_stack */ \
	movh r9, hi(tct_current_thread); \
	orr r9, lo(tct_current_thread); \
	ldm r9, r9, 0; /* dereference tct_current_thread */ \
	ldm r10, r9, TI_TASK; /* load pointer to task */ \
	ldm r9, r10, TASK_WHICH_TASK; \
	mov r11, sp; /* remember sp for restoring r9, r10, r11 */ \
	neq r0, r9, r0; \
	beq 1f; \
	/* we are on user stack, switch */ \
	stm sp, r10, TASK_USP; /* store USP */ \
	ldm sp, r10, TASK_KSP; /* load KSP */ \
	mov r0, 0; \
	stm r0, r10, TASK_WHICH_STACK; /* set which_stack to 0 */ \
	/* restore r9, r10, r11 */ \
	ldm r9, r11, -4; \
	ldm r10, r11, -8; \
	ldm r11, r11, -12; \
	/* jmp 2f; */ \
1: /* already on kernel stack */ \
	/* restore r9, r10 */
	ldm r9, sp, -4; \
	ldm r10, sp, -8; \
	/* no need to restore r11 */ \
2: /* now for sure on kernel stack */ 

/*	
long_breakpoint_handler: 
	ENSURE_TO_BE_ON_KERNEL_STACK; \
	addi sp, sp, -132; \
	call _save_syscall_frame; \
	mov r1, 32; \ 
	addi r2, sp, 4; \
	call asm_do_sig; \
	jmp _return_from_debug_exception 

*/

/**************************/
/* exception return paths */
/**************************/

/* return path for debug or non-debug exceptions */
#define EXCEPTION_RETURN_PATH(label, branch_to) \
label: \
	/* store pt_regs* in r2 */ \
	addi r2, sp, 4; \
	/* store 0 into r8 (syscall no) in pt_regs */ \
	mov r0, 0; \
	stm r0, sp, 36; \
	call manage_signals; \
	stm r1, sp, 8; \
	jmp branch_to

EXCEPTION_RETURN_PATH(_return_from_exception, _restore_and_return_exception)

EXCEPTION_RETURN_PATH(_return_from_debug, _restore_and_return_debug_exception)

/* ret_from_fork(unused, arg2, arg3, arg1, continuation) */
/* calls schedule_tail and then manage_signals */
/* returns to continuation(arg1, arg2, arg3) */
ENTRY(ret_from_fork)
	addi	sp, sp, -16
	stm 	r2, sp, 4
	stm	r3, sp, 8
	stm 	r4, sp, 12
	stm	r5, sp, 16
	call	schedule_tail
	ldm	r1, sp, 12
	mov	r2, 0
	/* call manage_signals (seems TODO) */
	ldm	r2, sp, 4
	ldm	r3, sp, 8
	ldm	lkr, sp, 16
	addi	sp, sp, 16
	rtn	lkr, sp, -4	 

/*********************************/
/* entry (wrapper) to syscalls   */
/*********************************/

ENTRY(sys_execve)
	mov 	r4, r7
	jmp 	tct_execve

ENTRY(sys_rt_sigsuspend_wrapper)
	/* save return address (lkr) to kernel stack */
	addi 	sp, sp, -4
	stm 	lkr, sp, 4
	/* store regs into 3rd argument */
	mov 	r3, r7
	call 	sys_rt_sigsuspend
	/* load lkr from kernel stack */
	ldm 	lkr, sp, 4
	addi 	sp, sp, 4
	rtn	lkr, sp, -4 	

ENTRY(sys_vfork)
	/* save lkr to kernel stack */
	addi 	sp, sp, -4
	stm 	lkr, sp, 4
	/* store regs into 1st argument */
	mov 	r1, r7
	/* store lkr into 2nd argument */
	mov 	r2, lkr
	call 	sys_tct_vfork
	/* load lkr from kernel stack */
	ldm 	lkr, sp, 4
	addi 	sp, sp, 4
	rtn	lkr, sp, -4 	

/* purpose of this wrapper: putting struct pt_regs* into 1st arg */
ENTRY(sys_sigreturn)
	/* save lkr to stack */
	addi 	sp, sp, -4
	stm 	lkr, sp, 4
	/* fix 1st arg */
	mov 	r1, r7
	call 	sys_sigreturn
	/* load ra from stack */
	ldm 	lkr, sp, 4
	addi 	sp, sp, 4
	rtn	lkr, sp, -4 	

ENTRY(sys_clone)
	/* save lkr to stack */
	addi 	sp, sp, -4
	stm 	lkr, sp, 4
	/* store lkr into 5th arg */
	mov 	r5, lkr
	call 	sys_tct_clone
	/* load lkr from stack */
	ldm 	lkr, sp, 4
	addi 	sp, sp, 4
	rtn	lkr, sp, -4 	

/* in IRQ we call a function between save and restore */
/* we therefore only save and restore the caller saved registers */
_long_interrupt_handler:
	addi 	sp, sp, -132
	stm	lkr, sp, 120
	call	_save_irq_frame
	
	/* workaround hardware hazard. */
	/* mfs r1, msr */ .word 0x18000000
	and	r1, r1, MSR_IRQ
	neq	r0, r1, 0
	/*beq	6f */
	
	




/*TODO: Wait for Control & Status Register */
	
_save_irq_frame:
	stm	r1, sp, 8
	stm	r2, sp, 12
	stm	r3, sp, 16
	stm	r4, sp, 20
	stm	r5, sp, 24
	stm	r6, sp, 28
	stm	r7, sp, 32
	stm	r8, sp, 36
	stm	r9, sp, 40
	stm	r10, sp, 44
	/* lkr (sp + 120) should have been written already */
	/* mfs	r0, elnk */ .word 0x1A000000
	stm	r0, sp, 124 
	
	movh 	r1, hi(kernel_mode)
	orr	r1, lo(kernel_mode)
	ldm	r2, r1, 0
	stm	r2, sp, 132
	mov	r2, PT_MODE_KERNEL
	stm	r2, r1, 0
	rtn	lkr, sp, -4

/* restore all caller saved registers saved in _save_irq_frame and return from exception */
_restore_irq_frame_and_return:
	ldm 	r2, sp, 132
	movh	r1, hi(kernel_mode)
	orr	r1, lo(kernel_mode)
	stm	r2, r1, 0

	ldm	r1, sp, 8
	ldm	r2, sp, 12
	ldm	r3, sp, 16
	ldm	r4, sp, 20
	ldm	r5, sp, 24
	ldm	r6, sp, 28
	ldm	r7, sp, 32
	ldm	r8, sp, 36
	ldm	r9, sp, 40
	ldm	r10, sp, 44
	ldm	lkr, sp, 120
	ldm	r0, sp, 124
	/* mts  elnk, r0*/	 .word 0x1A080000
	addi	sp, sp, 132	
	/* ertn */		 .word 0x10040000
		
_save_syscall_frame:
	stm	r1, sp, 8
	stm	r2, sp, 12
	stm	r3, sp, 16
	stm	r4, sp, 20
	stm	r5, sp, 24
	stm	r6, sp, 28
	stm	r7, sp, 32
	stm	r8, sp, 36
	stm	r9, sp, 40
	stm	r10, sp, 44
	stm	r11, sp, 48
	stm	r12, sp, 52
	stm	r13, sp, 56
	stm	r14, sp, 60
	stm	r15, sp, 64
	stm	r16, sp, 68
	stm	r17, sp, 72
	stm	r18, sp, 76
	stm	r19, sp, 80
	stm	r20, sp, 84
	stm	r21, sp, 88
	stm	r22, sp, 92
	stm	r23, sp, 96
	stm	r24, sp, 100
	stm	r25, sp, 104
	/* lkr (sp + 120) should have been written already */
	/* mfs	r0, elnk */ .word 0x1A000000
	stm	r0, sp, 124  

	movh	r11, hi(kernel_mode)
	orr	r11, lo(kernel_mode)
	ldm	r12, r11, 0
	stm	r12, sp, 132
	mov	r12, PT_MODE_KERNEL
	stm	r12, r11, 0
	rtn	lkr, sp, -4

/*************************/
/* syscall return paths  */
/*************************/

/* Restore all registers from syscall */
/* all interrupts are disabled upon entry */
/* we are on the kernel stack upon entry */

#define RETURN_FROM_SYSCALL_OR_EXCEPTION(label, addr, register, return_instr) \
label:	\
	ldm r2, sp, 132; \
	movh r1, hi(kernel_mode); \
	orr r1, lo(kernel_mode); \
	stm r2, r1, 0; \
	
	/* r9: scratch register */ \
	/* r10: current = current_thread_info()->task */ \
	/* r11: ksp backup */ \
	/* setup r10 = current */ \
	addi sp, sp, 132; \







ENTRY(resume)
	
_restore_and_return_exception:
_restore_and_return_debug_exception:



































































	
