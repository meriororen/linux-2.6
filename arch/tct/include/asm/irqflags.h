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
	asm volatile ( 
#if 0
		".word 0x18C03000			\n"
		"movh	r1, 0xdead			\n"
		"orr	r1, 0xbeef			\n"
		"neq	r0, r1, r3				\n"
		"bneq	1f				\n"
		".word 0x19001500			\n"
		"addi	r22, r22, 0x1			\n"
		".word 0x19021500			\n"
		"1:					\n"
#endif
		"/* mfs r0, msr*/ .word 0x18000000 	\n"
		" and	r0, r0, %0			\n"	
		" /* mts msr, r0*/ .word 0x18020000	\n"
		: 
		: "i" (~MSR_IRQ)
		: "R0"
#if 0
		, "R1", "R22", "R3"
#endif
	);
}

static inline unsigned long arch_local_irq_save(void)
{
	unsigned long flags;

	asm volatile (
		" /* mfs r0, msr */ .word 0x18000000	\n"
		" mov	%0, r0				\n"
		" and	r0, r0, %1			\n"
		" /* mts msr, r0 */ .word 0x18020000	\n"
		: "=r"(flags)
		: "i"(~MSR_IRQ) 
		: "R0"
	);
	return flags;
}

static inline unsigned long arch_local_save_flags(void)
{
	unsigned long flags;

	asm volatile (
		" /* mfs r0, msr */ .word 0x18000000	\n"
		" mov %0, r0				\n"
		: "=r" (flags)
		: 
		: "R0"
	); 
	return flags; 
}



static inline void arch_local_irq_enable(void)
{

	asm volatile(
		"mov	r0, 0x0				\n"
		".word 0x18820000			\n"
		" /* mfs r0, msr */ .word 0x18000000	\n"
		" orr	r0, %0				\n"
		" /* mts msr, r0 */ .word 0x18020000	\n"
		: 
		: "i"(MSR_IRQ)
		: "R0"
	);
}

static inline void arch_local_irq_restore(unsigned long flags)
{
	asm volatile(
		" mov r0, %0				\n"
		" /* mts msr, r0 */ .word 0x18020000	\n"
		: 
		: "r"(flags)
		: "R0"
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
