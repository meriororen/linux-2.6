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

/*
 * No exception vector yet
 */

ENTRY(system_call)
	/* break */
	/* store away r9, r10 so that we can use it here */
	stm r9, sp, 0	/* needed for various */
	stm r10, sp, -4	/* needed for current = current_thread_info()->task */
	stm r11, sp, -8	/* needed for user stack pointer, if switching */
	
	/* test if already on kernel stack: test current_thread_info->task->which_stack */
	movh r9, hi(tct_current_thread)
	orr r9, r9, lo(tct_current_thread)
	ldm r9, r9, 0 	/* dereference tct_current_thread */
	ldm r10, r9, TI_TASK /* load pointer to task */
	ldm r9, r10, TASK_WHICH_STACK /* reference to type of stack */

	mov r11, sp /* remember sp for restoring r9, r10, r11 */

	/* r9 now contains current task's which_stack */
	neq r0, r9, r0
	beq 1f

	/* we are on user stack, need to switch */
	stm sp, r10, TASK_USP /* store usp */
	ldm sp, r10, TASK_KSP /* load ksp */
	stm r0, r10, TASK_WHICH_STACK 

1: /* already on kernel stack */
	
	addi sp, sp, -132
	stm r11, sp, 116
	
	/* restore r9, r10, r11 */
	ldm r9, r11, 0
	ldm r10, r11, -4
	ldm r11, r11, -8

   /* save registers */
	stm ra, sp, 120
	call _save_syscall_frame
	
	/* r7 always holds the pointer to struct pt_regs */
	addi r7, sp, 4
	#addi r4, sp, 4
	
	/* r8 always holds the syscall number */
	/* check if syscall number is valid */
	mov r9, __NR_syscalls
	clt r0, r8, r9, 0 /* r8 < r9 ? r0 = 1 : r0 = 0 */
	beq .badsyscall
	movh r9, hi(sys_call_table) /* load address of syscall table */
	orr r9, r9, lo(sys_call_table)
	shl r10, r8, 2 
	add r9, r9, r10 /* add offset of syscall number to address */
	ldm r9, r9, 0 /* fetch address of syscall function */
	mov r0, r9 /* TODO: confirm this is right on TCT */
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
	/* store away r9, r10 so that we can use it here */
	

















	
