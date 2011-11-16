#ifndef _TCT_IRQFLAGS_H
#define _TCT_IRQFLAGS_H

static inline void arch_local_irq_disable(void)
{
	//unsigned int ie;
	asm volatile ( ""
		/*
		"rcsr %0, IE\n" \  // disabling irq via IE (LM32)
		"andi %0, %0, 0xfffe\n" \
		"wcsr IE, %0\n" \
		: "=r"(ie) \
		*/
	);
}

static inline unsigned long arch_local_save_flags(void)
{
	unsigned long ie;
	asm volatile ( ""
		/*
		"rcsr %0, IE\n" : "=r" 
		*/
	); 
	return ie; //retreive saved flags from IE
}

static inline void arch_local_irq_save(void)
{
	//unsigned int old_ie, new_ie;
	asm volatile (
		"" /* saving local irq flags into IE */
	);
}

static inline void arch_local_irq_enable(void)
{
	//unsigned int ie;
	asm volatile(
		"" /* enabling irq via IE */
	);
}

static inline void arch_local_irq_restore(unsigned long flags)
{
	//unsigned int ie;
	asm volatile("");
}

static inline int arch_irqs_disabled_flags(unsigned long flags)
{
	return ((flags & 0x1) == 0);
}

static inline int arch_irqs_disabled(void)
{
	return arch_irqs_disabled_flags(arch_local_save_flags());
}

#endif
