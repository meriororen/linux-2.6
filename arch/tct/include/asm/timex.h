/*
 * From Theobroma System's UCLINUX
 */



#ifndef _ASM_TIMEX_H
#define _ASM_TIMEX_H

#ifdef __KERNEL__

/* cannot use value supplied by bootloader because this value is used in an #if */
/* 100 MHz works as default value, even for 75 MHz bitstreams */
#define CLOCK_TICK_RATE		(100*1000*1000)

#endif /* __KERNEL__ */

typedef unsigned long cycles_t;
cycles_t get_cycles(void);

#endif
