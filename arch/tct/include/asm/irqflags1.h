/*
 * Based on:
 * arch/microblaze/include/asm/irqflags.h
 */

#ifndef _TCT_IRQFLAGS_H
#define _TCT_IRQFLAGS_H

#include <linux/types.h>
#include <asm/hw/registers.h>

static inline void arch_local_irq_disable(void)
{
	unsigned long tmp = 0;

	asm volatile ( 
		" mov %1, r5				\n"
		" /* mfs r5, msr*/ .word 0x18005000 	\n"
		" and	r5, r5, %0			\n"	
		" /* mts msr, r5*/ .word 0x18025000	\n"
		" mov r5, %1				\n"
		: 
		: "i" (~MSR_IRQ), "r"(tmp)
		: "memory"
	);
}

static inline unsigned long arch_local_irq_save(void)
{
	unsigned long flags;
	unsigned long tmp = 0;

	asm volatile (
		" mov %2, r5				\n"
		" /* mfs r5, msr */ .word 0x18005000	\n"
		" mov	%0, r5				\n"
		" and	r5, r5, %1			\n"
		" /* mts msr, r5 */ .word 0x18025000	\n"
		" mov r5, %2				\n"
		: "=r"(flags)
		: "i"(~MSR_IRQ), "r"(tmp)
		: "memory"
	);
	return flags;
}

static inline unsigned long arch_local_save_flags(void)
{
	unsigned long flags;
	unsigned long tmp = 0;

	asm volatile (
		" mov %1, r5				\n"
		" /* mfs r5, msr */ .word 0x18005000	\n"
		" mov %0, r5				\n"
		" mov r5, %1				\n"
		: "=r" (flags)
		: "r"(tmp)
		: "memory"
	); 
	return flags; 
}



static inline void arch_local_irq_enable(void)
{
	unsigned long tmp = 0;

	asm volatile(
		" mov %1, r5				\n"
		" /* mfs r5, msr */ .word 0x18005000	\n"
		" orr	r5, %0				\n"
		" /* mts msr, r5 */ .word 0x18025000	\n"
		" mov r5, %1				\n"
		: 
		: "i"(MSR_IRQ), "r"(tmp)
		: "memory"
	);
}

static inline void arch_local_irq_restore(unsigned long flags)
{
	unsigned long tmp = 0;	

	asm volatile(
		" mov %1, r5				\n"
		" mov r5, %0				\n"
		" /* mts msr, r5 */ .word 0x18025000	\n"
		" mov r5, %1				\n"
		: 
		: "r"(flags), "r"(tmp)
		: "memory"
	);
}

static inline int arch_irqs_disabled_flags(unsigned long flags)
{
	return ((flags & MSR_IRQ) == 0);
}

static inline int arch_irqs_disabled(void)
{
	return arch_irqs_disabled_flags(arch_local_save_flags());
}

#endif
