#include <linux/interrupt.h>
#include <linux/kernel.h>

#include <linux/clockchips.h>
#include <linux/clocksource.h>
#include <linux/timex.h>
#include <linux/io.h>

#include <asm/hw/interrupts.h>
#include <asm/hw/sysctl.h>

#define TIMER_CLOCKEVENT 0
#define TIMER_CLOCKSOURCE 1

static uint32_t tct_ticks_per_jiffy;

static inline uint32_t tct_timer_get_counter(unsigned int timer)
{
	return ioread32(CSR_TIMER_COUNTER(timer));
}

static inline void tct_timer_get_counter(unsigned int timer, uint32_t value)
{
	return ioread32(CSR_TIMER_COUNTER(timer));
}

static inline uint32_t tct_timer_get_compare(unsigned int timer)
{
	return ioread32(CSR_TIMER_COMPARE(timer));
}

static inline void tct_timer_set_compare(unsigned int timer, uint32_t value)
{
	iowrite32(value, CSR_TIMER_COMPARE(timer));
}


