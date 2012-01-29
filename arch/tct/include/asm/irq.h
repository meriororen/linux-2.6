#ifndef _TCT_ASM_IRQ_H
#define _TCT_ASM_IRQ_H

#include <asm/atomic.h>

#define NR_IRQS	4
#include <asm-generic/irq.h>

#define NO_IRQ 0

static inline void tct_irq_ack(unsigned int irq)
{
	uint32_t mask = (1<<irq);

	__asm__ __volatile__(
		"mov	r25, %0 				\n"
		"/*mts	irq, r25*/ .word 0x1A839000		\n"
		: 
		: "r"(mask)
		: "R25"
	); 
}

static inline uint32_t tct_irq_pending(void)
{
	uint32_t ip;

	__asm__ __volatile__(
		"/*mfs	r25, irq*/ .word 0x1A819000 	\n"
		"mov	%0, r25				\n"
		: "=r"(ip)
		: 
		: "R25"
	);
	return ip;
}
#endif
