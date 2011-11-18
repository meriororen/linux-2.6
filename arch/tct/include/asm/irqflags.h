/*
 * Based on:
 * arch/microblaze/include/asm/irqflags.h
 */

#ifndef _TCT_IRQFLAGS_H
#define _TCT_IRQFLAGS_H

#include <linux/types.h>
#include <asm/registers.h>

static inline void arch_local_irq_disable(void)
{
	unsigned long tmp;
	asm volatile ( 
		" mfs	%0, msr		\n"
		" nop			\n"
		" andi	%0, %0, %1	\n"
		" mts	msr, %0		\n"
		" nop			\n"
		: "=r"(tmp)
		: "i" (~MSR_IE)
		: "memory"
	);
}

static inline void arch_local_irq_save(void)
{
	unsigned int flags, tmp;
	asm volatile (
		" mfs	%0, msr		\n"
		" nop			\n"
		" andi	%1, %0, %2	\n"
		" mts	msr, %1		\n"
		" nop			\n"
		: "=r"(flags), "=r"(tmp)
		: "i"(~MSR_IE)
		: "memory"
	);
}

static inline unsigned long arch_local_save_flags(void)
{
	unsigned long flags;
	asm volatile (
		" mfs	%0, msr		\n"
		" nop			\n"
		: "=r" (flags)
		:
		: "memory"
	); 
	return flags; 
}



static inline void arch_local_irq_enable(void)
{
	unsigned long tmp;
	asm volatile(
		" mfs	%0, msr		\n"
		" nop			\n"
		" orr	%0, %0, %1	\n"
		" mts	msr, %0		\n"
		" nop			\n"
		: "=r"(tmp)
		: "i"(MSR_IE)
		: "memory"
	);
}

static inline void arch_local_irq_restore(unsigned long flags)
{
	asm volatile(
		" mts	 msr, %0	\n"
		" nop			\n"
		: 
		: "r"(flags)
		: "memory"
	);
}

static inline int arch_irqs_disabled_flags(unsigned long flags)
{
	return ((flags & MSR_IE) == 0);
}

static inline int arch_irqs_disabled(void)
{
	return arch_irqs_disabled_flags(arch_local_save_flags());
}

#endif
