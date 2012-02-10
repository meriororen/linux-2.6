#ifndef _TCT_ASM_IRQ_H
#define _TCT_ASM_IRQ_H

#include <asm/atomic.h>

#define NR_IRQS	5
#include <asm-generic/irq.h>

#define NO_IRQ 0

static inline void tct_irq_ack(unsigned int irq)
{
	uint32_t mask = (1<<irq);

	__asm__ __volatile__(
		"/*mfs	r3, irq*/ .word 0x1A803000	\n"
		"not	%0, %0				\n"
		"and	%0, %0, r3			\n"
		"mov	r0, %0 				\n"	
		"/*mts	irq, r0*/ .word 0x1A820000		\n": 
		: "r"(mask)
		: "R0", "R3"
	); 
}

static inline uint32_t tct_irq_pending(void)
{
	uint32_t irq;

	__asm__ __volatile__(
		"/*mfs	r0, irq*/ .word 0x1A800000 	\n"
		"mov	%0, r0				\n"
		: "=r"(irq)
		: 
		: "R0"
	);
	return irq;
}
#endif
