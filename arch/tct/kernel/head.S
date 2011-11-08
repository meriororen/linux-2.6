#include <linux/init.h>
#include <linux/linkage.h>
#include <asm/thread_info.h>

_INIT
ENTRY(_start)
	xor	r0, r0, r0 			/* clear r0 , not sure about this. */
	
	/* init stack pointer */
	movh	sp, hi(init_thread_union)
	orr	sp, sp, lo(init_thread_union)
	addi	sp, sp, THREAD_SIZE - 4

	/* terminate frame pointer */
	mov	fp, 0
	
	call	machine_early_init 		/* inside kernel/setup.c */
	jmp	start_kernel				
	
