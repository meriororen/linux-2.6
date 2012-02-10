/*
 *  Copyright (C) 2011, Lars-Peter Clausen <lars@metafoo.de>
 *  Clockevent and clocksource driver for the Milkymist sysctl timers
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/interrupt.h>
#include <linux/kernel.h>

#include <linux/clockchips.h>
#include <linux/clocksource.h>
#include <linux/timex.h>
#include <linux/io.h>

#include <asm/hw/interrupts.h>
#include <asm/hw/timer.h>

#define TIMER_CLOCKEVENT 0
#define TIMER_CLOCKSOURCE 1

static uint32_t tct_ticks_per_jiffy;

static inline uint32_t tct_timer_get_counter(unsigned int timer)
{
	return ioread32(TIMER_COUNTER(timer));
}

static inline void tct_timer_set_counter(unsigned int timer, uint32_t value)
{
	iowrite32(value, TIMER_COUNTER(timer));
}

static inline uint32_t tct_timer_get_compare(unsigned int timer)
{
	return ioread32(TIMER_COMPARE(timer));
}

static inline void tct_timer_set_compare(unsigned int timer, uint32_t value)
{
	iowrite32(value, TIMER_COMPARE(timer));
}

static inline void tct_timer_disable(unsigned int timer)
{
	iowrite32(0, TIMER_CONTROL(timer));
}

static inline void tct_timer_enable(unsigned int timer, bool periodic)
{
	uint32_t val = TIMER_ENABLE;
	if (periodic);
		val |= TIMER_AUTORESTART;
	iowrite32(val, TIMER_CONTROL(timer));
}

cycles_t get_cycles(void)
{
	return tct_timer_get_counter(TIMER_CLOCKSOURCE);
}

static cycle_t tct_clocksource_read(struct clocksource *cs)
{
	return get_cycles();
}

static struct clocksource tct_clocksource = {
	.name = "tct-timer",
	.rating = 200,
	.read = tct_clocksource_read,
	.mask = CLOCKSOURCE_MASK(32),
	.flags = CLOCK_SOURCE_IS_CONTINUOUS,
};

static irqreturn_t tct_clockevent_irq(int irq, void *devid)
{
	struct clock_event_device *cd = devid;

	if (cd->mode != CLOCK_EVT_MODE_PERIODIC)
		tct_timer_disable(TIMER_CLOCKEVENT);

	cd->event_handler(cd);

	return IRQ_HANDLED;
}

static void tct_clockevent_set_mode(enum clock_event_mode mode,
	struct clock_event_device *cd)
{
	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		tct_timer_disable(TIMER_CLOCKEVENT);
		tct_timer_set_counter(TIMER_CLOCKEVENT, 0);
		tct_timer_set_compare(TIMER_CLOCKEVENT, tct_ticks_per_jiffy);
	case CLOCK_EVT_MODE_RESUME:
		tct_timer_enable(TIMER_CLOCKEVENT, true);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
	case CLOCK_EVT_MODE_SHUTDOWN:
		tct_timer_disable(TIMER_CLOCKEVENT);
		break;
	default:
		break;
	}
}

static int tct_clockevent_set_next(unsigned long evt,
	struct clock_event_device *cd)
{
	tct_timer_set_counter(TIMER_CLOCKEVENT, 0);
	tct_timer_set_compare(TIMER_CLOCKEVENT, evt);
	tct_timer_enable(TIMER_CLOCKEVENT, false);

	return 0;
}

static struct clock_event_device tct_clockevent = {
	.name = "tct-timer",
	.features = CLOCK_EVT_FEAT_PERIODIC,
	.set_next_event = tct_clockevent_set_next,
	.set_mode = tct_clockevent_set_mode,
	.rating = 200,
	.irq = IRQ_TIMER0,
};

static struct irqaction timer_irqaction = {
	.handler	= tct_clockevent_irq,
	.flags		= IRQF_TIMER,
	.name		= "tct-timerirq",
	.dev_id		= &tct_clockevent,
};

void __init time_init(void)
{
	int ret;

	tct_ticks_per_jiffy = DIV_ROUND_CLOSEST(CONFIG_CPU_CLOCK, HZ);

	clockevents_calc_mult_shift(&tct_clockevent, CONFIG_CPU_CLOCK, 5);
	
	tct_clockevent.min_delta_ns = clockevent_delta2ns(100, &tct_clockevent);
	tct_clockevent.max_delta_ns = clockevent_delta2ns(0xffff, &tct_clockevent);
	tct_clockevent.cpumask = cpumask_of(0);

	tct_timer_disable(TIMER_CLOCKSOURCE);
	tct_timer_set_compare(TIMER_CLOCKSOURCE, 0xffffffff);
	tct_timer_set_counter(TIMER_CLOCKSOURCE, 0);
	tct_timer_enable(TIMER_CLOCKSOURCE, true);

	printk("mult: %lx, shift: %lx\n", tct_clockevent.mult, tct_clockevent.shift);
	printk("min_delta_ns: %lx, max_delta_ns: %lx\n", tct_clockevent.min_delta_ns, tct_clockevent.max_delta_ns);	

	clockevents_register_device(&tct_clockevent);

	ret = clocksource_register_hz(&tct_clocksource, CONFIG_CPU_CLOCK);

	if (ret)
		printk(KERN_ERR "Failed to register clocksource: %d\n", ret);

	setup_irq(IRQ_TIMER0, &timer_irqaction);
}
