#ifndef _TCT_ASM_IRQ_H
#define _TCT_ASM_IRQ_H

#include <asm/atomic.h>

#define NR_IRQS	32
#include <asm-generic/irq.h>

#define NO_IRQ 0

static inline void tct_irq_ack(unsigned int irq)
{
	uint32_t mask = (1<<irq);
	__asm__ __volatile__(
		"MOV	R0, %0 				\n"
		"/*MTS	IRQ, R0*/ .word 0x1A820000	\n"
		: 
		: "r"(mask)
		:
	); 
}

static inline uint32_t tct_irq_pending(void)
{
	uint32_t ip;
	__asm__ __volatile__(
		"/*MFS	R0, IRQ*/ .word 0x1A800000 	\n"
		"MOV	%0, R0				\n"	
		: "=r"(ip)
		:
		:
	);
	return ip;
}
#endif
