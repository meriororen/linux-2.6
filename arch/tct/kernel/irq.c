#include <linux/atomic.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/kernel_stat.h>
#include <linux/seq_file.h>
#include <linux/types.h>
#include <linux/of_fdt.h>
#include <linux/module.h>

atomic_t irq_err_count;

static inline uint32_t tct_pic_get_irq_mask(struct irq_data *data)
{
	return (uint32_t)irq_data_get_irq_chip_data(data);
}

static void tct_pic_irq_mask(struct irq_data *data)
{
	uint32_t mask = tct_pic_get_irq_mask(data);
	uint32_t im;

	/* need status and ctl registers */
}

static struct irq_chip tct_irq_chip = {
	.name		= "TCT PIC",
	.irq_ack	= tct_pic_irq_ack,
	.irq_mask	= tct_pic_irq_mask,
	.irq_mask_ack	= tct_pic_irq_mask_ack,
	.irq_unmask	= tct_pic_irq_unmask,
}

void __init init_IRQ(void)
{
	unsigned int irq;
	
	local_irq_disable();
	/* need status and ctl registers */
	
	
}
int arch_show_interrupts(struct seq_file *p, int prec)
{
	seq_printf(p, "%*s: %10u\n", prec, "ERR", atomic_read(&irq_err_count));
	return 0;
}

