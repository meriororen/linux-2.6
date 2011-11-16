#ifndef _LM32_ASM_IRQ_H
#define _LM32_ASM_IRQ_H

#include <asm/atomic.h>

#define NR_IRQS	32
#include <asm-generic/irq.h>

#define NO_IRQ 0

static inline uint32_t tct_irq_pending(void)
{
	uint32_t ip;
	__asm__ __volatile__(""); //rcsr 0, IP
	return ip;
}

static inline void tct_irq_ack(unsigned int irq)
{
	uint32_t mask = (1<<irq);
	__asm__ __volatile__(""); //wcsr IP, 0
}

#endif
