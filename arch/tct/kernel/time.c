#include <linux/interrupt.h>
#include <linux/kernel.h>

#include <linux/clockchips.h>
#include <linux/clocksource.h>
#include <linux/timex.h>
#include <linux/io.h>

#include <asm/hw/interrupts.h>


#define TIMER_CLOCKEVENT 0
#define TIMER_CLOCKSOURCE 1


static uint32_t tct_ticks_per_jiffy;

static inline uint32_t tct_timer_get_counter(unsigned int timer)
{
	printk("%s: called\n", __func__);
	return 0;
}

static inline void tct_timer_set_counter(unsigned int timer, uint32_t value)
{
	printk("%s: called\n", __func__);
}

static inline uint32_t tct_timer_get_compare(unsigned int timer)
{
	printk("%s: called\n", __func__);
	return 0;
}
static inline void tct_timer_set_compare(unsigned int timer, uint32_t value)
{
	printk("%s: called\n", __func__);
}

static inline void tct_timer_disable(unsigned int timer)
{
	printk("%s: called\n", __func__);
}

cycles_t get_cycles(void)
{
	printk("%s: called\n", __func__);
}

static cycle_t tct_clocksource_read(struct clocksource *cs)
{
	printk("%s: called\n", __func__);
	return get_cycles();
}

static struct clocksource tct_clocksource = {
};

static irqreturn_t tct_clockevent_irq(int irq, void *devid)
{
	printk("%s: called\n", __func__);
	return IRQ_HANDLED;
}

static void tct_clockevent_set_mode(enum clock_event_mode mode,
	struct clock_event_device *cd)
{
	printk("%s: called\n", __func__);
}

static int tct_clockevent_set_next(unsigned long evt,
	struct clock_event_device *cd)
{
	printk("%s: called\n", __func__);
return 0;
}

static struct clock_event_device tct_clockevent = {

};

static struct irqaction timer_irqaction = {

};

void __init time_init(void)
{
	printk("%s: called\n", __func__);
}
