#include <linux/linkage.h>
#include <asm/thread_info.h>
#include <linux/errno.h>
#include <asm/entry.h>
#include <asm/asm-offsets.h>
#include <asm/registers.h>
#include <asm/unistd.h>
#include <asm/percpu.h>
#include <asm/signal.h>




ENTRY(_interrupt)
	
ENTRY(_reset)
	jmp	_start
ENTRY(_user_exception)
	stm	r1, r0, sp
	stm	
/* ENTRY(_debug_exception) */
/* ENTRY(_break)	   */
/* ENTRY(_switch_to)	   */
ENTRY(ret_from_fork)
ENTRY(ret_to_user)
