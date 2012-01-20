#include <linux/init.h>
#include <linux/linkage.h>
#include <asm/thread_info.h>


__INIT
ENTRY(_start)	
	jmp	1f
	jmp	_scall
	jmp	_cpu
	jmp	_mmu
	jmp	_irq

_scall:
	halt
_cpu:
	halt
_mmu:
	halt
_irq:
	halt

1:
	
	/* init stack pointer, see init_thread_union in kernel/init_task.c */
	movh	sp, hi(init_thread_union)
	orr	sp, lo(init_thread_union)
	addi	sp, sp, THREAD_SIZE - 4
	
	/* clear IMASK  */
	mov	r0, 0x000F
	/*mts 	r0, imask*/ .word 0x1A420000	

	/* r1 = hardware parameters,
	   r2 = initrd start,
	   r3 = initrd end,
	*/

	movh	r1, 0x7fe
	orr	r1, 0xf000
	movh	r2, 0x80
	orr	r2, 0x0000
	movh	r3, 0x100
	orr	r3, 0x0000


	/* disable mmu */
	/*mfs	r0, msr */ .word 0x18000000
	and	r0, r0, 0xfd
	/*mts	msr, r0 */ .word 0x18020000

	call	machine_early_init 		/* inside kernel/setup.c */
	jmp	start_kernel				
	
