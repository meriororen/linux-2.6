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
	__asm__ __volatile__(
		" /*mfs	r0, msr*/ .word 0x18000000	\n"
		" not r0, r0				\n"
		" nor r0, r0, %0			\n"
		:  
		: "r"(mask)
		:
	);
}

static void tct_pic_irq_unmask(struct irq_data *data)
{
	//uint32_t mask = tct_pic_get_irq_mask(data);
	
	__asm__ __volatile__(
		""
	);
}

static void tct_pic_irq_ack(struct irq_data *data)
{
	//uint32_t mask = tct_pic_get_irq_mask(data);

	__asm__ __volatile__(" /**/");
}

static void tct_pic_irq_mask_ack(struct irq_data *data)
{
	//uint32_t mask = tct_pic_get_irq_mask(data);

	__asm__ __volatile__ (
		""
	);
}




static struct irq_chip tct_irq_chip = {
	.name		= "LM32 PIC",
	.irq_ack	= tct_pic_irq_ack,
	.irq_mask	= tct_pic_irq_mask,
	.irq_mask_ack	= tct_pic_irq_mask_ack,
	.irq_unmask	= tct_pic_irq_unmask,
};

void __init init_IRQ(void)
{
//	unsigned int irq;
	
	printk("%s: called\n", __func__);
	local_irq_disable();
	/* need status and ctl registers */
	
	
}
int arch_show_interrupts(struct seq_file *p, int prec)
{
	printk("%s: called\n", __func__);
	seq_printf(p, "%*s: %10u\n", prec, "ERR", atomic_read(&irq_err_count));
	return 0;
}

/*
 * irq_create_of_mapping - Hook to resolve OF irq specifier into a linux irq number
 *
 * Currently the mapping mechanism is trivial; simple flat hwirq numbers are mapped
 * 1:1 onto Linux irq numbers. Cascade irq controllers are not supported.
 *
 */
unsigned int irq_create_of_mapping(struct device_node *controller,
				   const u32 *intspec, unsigned int intsize)
{
	return intspec[0];
}
EXPORT_SYMBOL_GPL(irq_create_of_mapping);

asmlinkage void asm_do_IRQ(unsigned int irq, struct pt_regs *regs)
{
	struct pt_regs *old_regs = set_irq_regs(regs);

	irq_enter();
	generic_handle_irq(irq);
	irq_exit();

	set_irq_regs(old_regs);
}

