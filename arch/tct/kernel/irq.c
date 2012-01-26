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
	uint32_t irq;
	unsigned long tmp = 0;

	__asm__ __volatile__(
		"mov %2, r25			\n"
		".word 0x1A819000		\n" /* mfs r0, irq */
		"mov %0, r25			\n"
		"not %0, %0			\n"
		"orr %0, %0, %1			\n"
		"not %0, %0			\n"
		"mov r25, %0			\n"
		".word 0x1A839000		\n"
		"mov r25, %2			\n"
		: "=&r"(irq)
		: "r"(mask), "r"(tmp)
	);
}

static void tct_pic_irq_unmask(struct irq_data *data)
{
	uint32_t mask = tct_pic_get_irq_mask(data);
	unsigned long tmp = 0;
	uint32_t irq;

	__asm__ __volatile__(
		"mov %2, r25			\n"
		".word 0x1A819000		\n"
		"mov %0, r25			\n"
		"orr %0, %0, %1			\n"
		"mov r25, %0			\n"
		".word 0x1A839000		\n"
		"mov r25, %2			\n"
		: "=&r"(irq)
		: "r"(mask), "r"(tmp)
	);
}

static void tct_pic_irq_ack(struct irq_data *data)
{
	uint32_t mask = tct_pic_get_irq_mask(data);
	unsigned long tmp = 0;

	__asm__ __volatile__(
		"mov %1, r25			\n"
		"mov r25, %0			\n"
		".word 0x1A439000		\n"
		"mov r25, %1			\n"
		: 
		: "r"(mask), "r"(tmp)
	);
}

static void tct_pic_irq_mask_ack(struct irq_data *data)
{
	uint32_t mask = tct_pic_get_irq_mask(data);
	unsigned long tmp = 0;
	uint32_t irq;

	__asm__ __volatile__ (
		"mov %2, r25			\n"
		".word 0x1A819000		\n"
		"mov %0, r25			\n"
		"not %0, %0			\n"
		"orr %0, %0, %1			\n"
		"not %0, %0			\n"
		"mov r25, %0			\n"
		".word 0x1A839000		\n"
		"mov r25, %1			\n"
		"mov r25, %2			\n"
		: "=&r"(irq)
		: "r"(mask), "r"(tmp)
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
	unsigned int irq;
	unsigned long tmp = 0;

	local_irq_disable();
	__asm__ __volatile__(
		"mov %0, r25			\n"
		"mov r25, 0x0			\n"
		".word 0x1A839000		\n"
		"mov r25, %0			\n"
		:
		: "r"(tmp)
	);	
	
	for (irq = 0; irq < NR_IRQS; irq++) {
		irq_set_chip(irq, &tct_irq_chip);
		irq_set_chip_data(irq, (void *)(1 << irq));
		irq_set_handler(irq, handle_level_irq);
	}

}
int arch_show_interrupts(struct seq_file *p, int prec)
{
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
