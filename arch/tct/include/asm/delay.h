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
		"and r2, %0, 1\n"
		"equ r0, r2, 0\n" /* jump over next instruction */
		"bneq	1f\n"
		"addi %0, %0, -1\n"
		"1:	and r2, %0, 2\n"
		"equ r0, r2, 0\n" /* jump over next instruction */
		"bneq	2f\n"
		"addi %0, %0, -2\n"
		"2:	and r2, %0, 4\n"
		"equ r0, r2, 0\n" /* jump over next instruction */
		"bneq 3f\n"
		"addi %0, %0, -4\n"
		"3:	addi %0, %0, -1\n" /* loop */ 
		"addi %0, %0, -1\n"
		"addi %0, %0, -1\n"
		"addi %0, %0, -1\n"
		"addi %0, %0, -1\n"
		"addi %0, %0, -1\n"
		"addi %0, %0, -1\n"
		"addi %0, %0, -1\n"
		"equ r0, %0, 0\n"
		"bneq 3b\n"
		: "=r"(loops)
		: "0"(loops)
		: "R2"
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
