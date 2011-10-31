#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init_task.h>
#include <linux/mqueue.h>
#include <linux/hadirq.h>

#include <asm/thread_info.h>
#include <asm/uaccess.h>

static struct signal_struct init_signals = INIT_SIGNALS(init_signals);
static struct sighand_struct init_sighand = INIT_SIGHAND(init_sighand);

union thread_union init_thread_union __init_task_data =
{
{
	.task =	&init_task,
	.exec_domain = &default_exec_domain,
	.flags = 0,
	.cpu = 0,
	.preempt_count = INIT_PREEMPT_COUNT,
	.restart_block = {
		.fn = do_no_restart_syscall,
	},
	.addr_limit = KERNEL_DS,
}
};

/*
 * Task struct of init
 *
 */
struct task_struct init_task = INIT_TASK(init_task);
EXPORT_SYMBOL(init_task);
