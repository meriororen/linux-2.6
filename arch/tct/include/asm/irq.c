#ifndef _TCT_ASM_IRQ_H
#define _TCT_ASM_IRQ_H

//#include <asm/atomic.h>

#define NR_IRQS	32
//#include <asm-generic/irq.h>

#define NO_IRQ 0

typedef unsigned long uint32_t;

static inline uint32_t tct_irq_pending(void)
{
	uint32_t ie;
	__asm__ __volatile__(
		"/*MFS	MSR, R0*/ .word 0x18000000	\n"
		" MOV	%0, R0 				\n"
		: "=r"(ie)
		: /* no input */
		: "memory"
	); 
	return ie;
}

static inline void tct_irq_ack(unsigned int irq)
{
	uint32_t mask = (1<<irq);
	__asm__ __volatile__(
		"MOV	R0, %0 				\n"
		"/*MTS	MSR, R0*/ .word 0x18020000	\n"
		: 
		: "r"(mask)
		: "memory"		
	); 
}


#endif
