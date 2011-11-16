/*
 * Based on:
 * include/asm-mips/delay.h
 */

#ifndef __TCT_DELAY_H
#define __TCT_DELAY_H

static inline void __delay(unsigned long loops)
{
	/* get a realistic delay by using add more often than branch
	 * (branch needs 4 cycles if taken, add only one) */
	asm volatile(
		""
	);
}

#include <linux/param.h>	/* needed for HZ */

/*
 * Use only for very small delays ( < 1 msec ). Should probably use a
 * lookup table. As the multiplications take much too long with short delays.
 * This is a "reasonable" implementation, though (and the first constant mul-
 * tiplications gets optimized away if the delay is a constat)
 */
static inline void udelay(unsigned long usecs)
{
	extern unsigned long loops_per_jiffy;
	__delay(usecs * loops_per_jiffy / (1000000 / HZ));
}

#endif
