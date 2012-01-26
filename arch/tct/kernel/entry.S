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

.section 	.head.text, "ax"
	.org 0x0
	jmp	_reset
	jmp	_scall_handler
	jmp	_exception_handler
	jmp	_mmu_handler
	jmp	_interrupt_handler


.section ".exception.text", "ax"

ENTRY(_reset)
	jmp _start
ENTRY(_exception_handler)
	addi	sp, sp, -152
	stm  	lkr, sp, 108
	call 	_save_syscall_frame
	/* mfs 	r1, esr*/ .word 0x19801000 /* esr in r1 */
	addi 	r2, sp, 4 /* pt_regs in r2 */
	call 	asm_do_sig
	jmp	_return_from_exception
	
ENTRY(_mmu_handler)
	halt
ENTRY(_interrupt_handler)
	halt	
ENTRY(_scall_handler)
	halt

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
	/* lkr (sp + 108) should have been written already */
	/* mfs	r0, elkr */ .word 0x1A000000
	stm	r0, sp, 144  

	movh	r11, hi(kernel_mode)
	orr	r11, lo(kernel_mode)
	ldm	r12, r11, 0
	stm	r12, sp, 152
	mov	r12, PT_MODE_KERNEL
	stm	r12, r11, 0
	rtn	lkr, sp, -4

_return_from_exception:
	halt
	/* store pt_regs in r2 */
	addi	r2, sp, 4
	call	manage_signals
	stm	r1, sp, 8 /* return value of manage_signals into pt_regs */
	ldm	r2, sp, 152
	movh	r1, hi(kernel_mode)
	orr	r1, lo(kernel_mode)
	stm	r2, r1, 0
	
	/* prepare switch to user stack, but keep kernel stack pointer in r11 */
	/* r9: scratch register */
	/* r10: current = current_thread_info()->task */
	/* r11: ksp backup */
	/* setup r10 = current */

	addi	sp, sp, 152

ENTRY(ret_from_fork) /* kernel/process.c */
ENTRY(sys_clone)
ENTRY(sys_vfork)
ENTRY(sys_execve) 
ENTRY(resume) /* kernel/sched.c */
	addi	sp, sp, -152
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
	
