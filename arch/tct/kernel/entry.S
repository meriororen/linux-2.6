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
	addi	sp, sp, -156
	stm  	lkr, sp, 108
	call 	_save_syscall_frame
	/* mfs 	r1, esr*/ .word 0x19801000 /* esr in r1 */
	addi 	r2, sp, 4 /* pt_regs in r2 */
	call 	asm_do_sig
	jmp	_return_from_exception
	
ENTRY(_mmu_handler)
	halt
ENTRY(_interrupt_handler)
	jmp	_long_interrupt_handler
	
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
	stm	r12, sp, 156
	mov	r12, PT_MODE_KERNEL
	stm	r12, r11, 0
	rtn	lkr, sp, -4

_return_from_exception:
	/* store pt_regs in r2 */
	addi	r2, sp, 4
	call	manage_signals
	stm	r1, sp, 8 /* return value of manage_signals into pt_regs */
	ldm	r2, sp, 156
	movh	r1, hi(kernel_mode)
	orr	r1, lo(kernel_mode)
	stm	r2, r1, 0
	
	/* prepare switch to user stack, but keep kernel stack pointer in r11 */
	/* r9: scratch register */
	/* r10: current = current_thread_info()->task */
	/* r11: ksp backup */
	/* setup r10 = current */

	addi	sp, sp, 156

ENTRY(ret_from_fork) /* kernel/process.c */
ENTRY(sys_clone)

_long_interrupt_handler:
	addi	sp, sp, -156
	stm	lkr, sp, 108
	call	_save_irq_frame
	
	/* Workaround hardware hazard. Sometimes the interrupt handler is entered
	 * although interrupts are disabled */
	/*mfs	r1, msr*/ .word 0x18001000
	movh	r7, 0x10
	orr	r7, 0x0
	and	r0, r1, r7		/*and r0, r1, 0x100000 is impossible*/	
	equ	r1, r1, 0
	bneq	5f

	/*mfs	r3, irq*/ .word 0x1A803000
	/*mfs	r4, imask*/ .word 0x1A404000
	mov	r1, 0
	and	r3, r3, r4
	equ	r0, r3, 0
	bneq	4f
	
	and	r4, r3, 0xff
	neq	r0, r4, 0
	bneq	1f
	shr	r3, r3, 8, 0x0
	addi	r1, r1, 8
1:	
	and	r4, r3, 0xf
	neq	r0, r4, 0
	bneq	2f
	shr	r3, r3, 4, 0x0
	addi	r1, r1, 4
2:	
	and	r4, r3, 0x3
	neq	r0, r4, 0
	bneq	3f
	shr	r3, r3, 2, 0x0
	addi	r1, r1, 2
3:	
	and	r4, r3, 0x1
	neq	r0, r4, 0
	bneq	4f
	addi	r1, r1, 1
4:
	addi	r2, sp, 4
	call	asm_do_IRQ
	addi	r1, sp, 4
	call	manage_signals_irq
5:
	jmp	_restore_irq_frame_and_return


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
	/* mfs	r0, elkr */ .word 0x1A000000
	stm	r0, sp, 144
	
	movh	r1, hi(kernel_mode)
	orr	r1, lo(kernel_mode)
	ldm	r2, r1, 0
	stm	r2, sp, 156
	mov	r2, PT_MODE_KERNEL
	stm	r2, r1, 0 
	rtn	lkr, sp, -4

_restore_irq_frame_and_return:
	ldm	r2, sp, 156
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
	ldm	lkr, sp, 108
	ldm	r0, sp, 144
	/*mts	elkr, r0*/ .word 0x1A020000
	addi	sp, sp, 156
	/*ertn*/ .word 0x10040000

ENTRY(sys_vfork)
ENTRY(sys_execve) 
ENTRY(resume) /* kernel/sched.c */
	addi	sp, sp, -156
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
	
