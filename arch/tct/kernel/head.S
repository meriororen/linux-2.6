#include <linux/init.h>
#include <linux/linkage.h>
#include <asm/thread_info.h>


__INIT
ENTRY(_start)
	/* init stack pointer, see init_thread_union in kernel/init_task.c */
	movh	sp, hi(init_thread_union)
	orr	sp, lo(init_thread_union)
	addi	sp, sp, THREAD_SIZE - 4
	
	
	/* clear IMASK  */
	xor	r0, r0, r0
	/*mts 	r0, imask*/ .word 0x1A420000

	/* r1 = cmdline,
	   r2 = initrd start,
	   r3 = initrd end,
	*/
	movh	r1, 0x7ff
	orr	r1, 0xf000
	movh	r2, 0x80
	orr	r2, 0x0000
	movh	r3, 0x100
	orr	r3, 0x0000


	/* disable mmu */
	/*mfs	r0, msr */ .word 0x18000000
	and	r0, r0, 0xfd
	/*mts	msr, r0 */ .word 0x18020000

	call	machine_early_init
	jmp	start_kernel				
	
