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
	printk("disable(mask): %d\n", data->irq);
	uint32_t mask = tct_pic_get_irq_mask(data);
	uint32_t r_irq;

	__asm__ __volatile__( 							// we want to mask b0_0100 (IRQ_TIMER0)
		".word 0x1A400000		\n" /* mfs r0, imask */     
		"mov %0, r0			\n" //r_irq = imask 	   e.g  b0_0100 	
		"inv %0, %0			\n" //r_irq = ~(r_irq)		b1_1011	
		"orr %0, %0, %1			\n" //r_irq = r_irq | mask	b1_1011 | b0_0100 (IRQ_TIMER0) = b1_1111
		"inv %0, %0			\n" //r_irq = ~(r_irq)		b0_0000
		"mov r0, %0			\n" //imask = r_irq		imask = b0_00000
		".word 0x1A420000		\n" /* mts imask, r0*/
		: "=&r"(r_irq)
		: "r"(mask)
		: "R0"
	);
}

static void tct_pic_irq_unmask(struct irq_data *data)
{
	printk("enable(unmask): %d\n", data->irq);
	uint32_t mask = tct_pic_get_irq_mask(data);
	uint32_t r_irq;

	__asm__ __volatile__(							// we want to unmask b0_0100 (IRQ_TIMER0)
		"/*mfs r0, imask*/ .word 0x1A400000	\n"			
		"mov %0, r0				\n"			//b0_0000
		"orr %0, %0, %1				\n"			//b0_0000 | b0_0100 = b0_0100
		"mov r0, %0				\n"			// imask = b0_0100	
		"/*mts imask, r0*/ .word 0x1A420000	\n"
		: "=&r"(r_irq)
		: "r"(mask)
		: "R0"
	);
}

static void tct_pic_irq_ack(struct irq_data *data)
{
	printk("ack: %d\n", data->irq);
	uint32_t mask = tct_pic_get_irq_mask(data);

	__asm__ __volatile__(							//we want to ack (set IRQ bit to 0) of IRQ_TIMER0
		"/* mfs r3, irq	*/ .word 0x1A803000	\n"			//mask = b0_0100, irq = b1_1100
		"inv %0, %0			\n"				//b1_1011
		"and %0, %0, r3			\n"				//b1_1011 & b1_1100 = b1_1000
		"mov r0, %0			\n"				//irq = b1_1000
		".word 0x1A820000		\n"
		: 
		: "r"(mask)
		: "R0", "R3"
	);
}

static void tct_pic_irq_mask_ack(struct irq_data *data)
{
	printk("disable and ack: %d\n", data->irq);
	uint32_t mask = tct_pic_get_irq_mask(data);
	uint32_t irq;

	__asm__ __volatile__ (							//we want to disable (set imask bit to 0)
		".word 0x1A400000		\n"				//and ack (set irq bit to 0) of IRQ_TIMER0
		"mov %0, r0			\n"				//b1_1111	mask = b0_0100, imask = b1_1111, irq=b1_0100
		"inv %0, %0			\n"				//b0_0000
		"orr %0, %0, %1			\n"				//b0_0000 | b0_0100 = b0_0100 	
		"inv %0, %0			\n"				//b1_1011
		"mov r0, %0			\n"				//imask = b1_011
		".word 0x1A420000		\n"
		"/* mfs r3, irq	*/ .word 0x1A803000	\n"			//b1_0100
		"inv %1, %1			\n"				//b1_1011
		"and %1, %1, r3			\n"				//b1_1011 & b1_0100 = b1_0000
		"mov r4, %1			\n"				//irq = b1_1111
		".word 0x1A824000		\n"
		: "=&r"(irq)
		: "r"(mask)
		: "R0", "R3", "R4"
	);
}


static struct irq_chip tct_irq_chip = {
	.name		= "lm32-pic",
	.irq_ack	= tct_pic_irq_ack,
	.irq_mask	= tct_pic_irq_mask,
	.irq_mask_ack	= tct_pic_irq_mask_ack,
	.irq_unmask	= tct_pic_irq_unmask,
};

void __init init_IRQ(void)
{
	unsigned int irq;

	local_irq_disable();
	__asm__ __volatile__(
		"mov r0, 0x0			\n"
		".word 0x1A420000		\n"
		:
		:
		: "R0" 
	);	
	
	printk("init_irq...\n");

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
	printk("%s->r10 = %lx | elkr = %lx\n", __func__, regs->r10, regs->elkr);

#if 0
	if(irq == 2){
		asm volatile(
			"movh r0, 0xdeed	\n"
			"orr	r0, 0xbeed	\n"
			".word 0x18C20000	\n"
			:::"R0"
		);
	}
#endif
	irq_enter();
	generic_handle_irq(irq);
	irq_exit();

	set_irq_regs(old_regs);
}
