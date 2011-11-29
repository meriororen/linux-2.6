#include <linux/init.h>
#include <linux/linkage.h>
#include <asm/thread_info.h>

__INIT
ENTRY(_start)
	
	/* init stack pointer, see init_thread_union in kernel/init_task.c */
	movh	sp, hi(init_thread_union)
	orr	sp, lo(init_thread_union)
	addi	sp, sp, THREAD_SIZE - 4
	

	call	machine_early_init 		/* inside kernel/setup.c */
	jmp	start_kernel				
	
