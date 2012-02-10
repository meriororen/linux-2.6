/*
 * (C) Copyright 2007
 *     Theobroma Systems <www.theobroma-systems.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Based on
 *
 * linux/arch/m68knommu/kernel/time.c
 *
 *  Copyright (C) 1991, 1992, 1995  Linus Torvalds
 */

#include <linux/errno.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/profile.h>
#include <linux/time.h>
#include <linux/timex.h>
#include <linux/interrupt.h>

#include <asm/uaccess.h>

#include <asm/irq.h>
#include <asm/hw/tct-board.h>
#include <asm/irq_regs.h>
#include <asm/setup.h>

TCTTag_Timer_t tcttag_timer;

typedef struct TCT_timer {
  volatile unsigned int  Status;
  volatile unsigned int  Control;
  volatile unsigned int  Period;
  volatile unsigned int  Snapshot;
} TCT_timer_t;

static TCT_timer_t* tct_core_timer_reg = NULL;

cycles_t tct_cycles = 0;

static irqreturn_t timer_interrupt(int irq, void *timer_idx);

/* irq action description */
static struct irqaction tct_core_timer_irqaction = {
	.name = "TCT Timer Tick",
	.flags = IRQF_DISABLED,
	.handler = timer_interrupt,
};

void tct_systimer_ack(void)
{
	if(likely(tct_core_timer_reg)) {
		/* update virtual cycle counter */
		tct_cycles += tct_core_timer_reg->Period;

		/* ack interrupt in timer control */
		tct_core_timer_reg->Status = 0;
		/* ack interrupt */
		tct_irq_ack(IRQ_TIMER0);
	}
}

/*
 * timer_interrupt() needs to call the "do_timer()"
 * routine every clocktick
 */
static irqreturn_t timer_interrupt(int irq, void *arg)
{
	tct_systimer_ack();

	write_seqlock(&xtime_lock);

	do_timer(1);

	if (current->pid)
		profile_tick(CPU_PROFILING);

	write_sequnlock(&xtime_lock);
	return(IRQ_HANDLED);
}

void set_timer_hw(void)
{
	tcttag_timer.name = "LM32-timer0";
	tcttag_timer.addr = 0x10001000;
	tcttag_timer.wr_tickcount = 1;
	tcttag_timer.rd_tickcount = 1;
	tcttag_timer.start_stop_control = 1;
	tcttag_timer.counter_width = 32;
	tcttag_timer.reload_ticks = 20;
	tcttag_timer.irq = IRQ_TIMER0;
	tcttag_timer.reserved0 = 0;
	tcttag_timer.reserved1 = 0;
	tcttag_timer.reserved2 = 0;
}

void tct_systimer_program(int periodic, cycles_t cyc)
{
	if( likely(tct_core_timer_reg) )
	{
		/* stop timer */
		tct_core_timer_reg->Control = (1 << 3);  /* STOP */
		/* reset/configure timer */
		tct_core_timer_reg->Status = 0;
		tct_core_timer_reg->Period = cyc;
		/* start timer */
		tct_core_timer_reg->Control = (
				(1 << 0) |  /* enable interrupts */ 
				(periodic << 1) |  /* single or periodic (CONT) */ 
				(1 << 2));  /* START */
	}
}

void time_init(void)
{
	set_timer_hw();

	xtime.tv_sec = 0;
	xtime.tv_nsec = 0;

	wall_to_monotonic.tv_sec = -xtime.tv_sec;

	/* timer registers */
	tct_core_timer_reg = tcttag_timer.addr;

	tct_systimer_program(1, CONFIG_CPU_CLOCK / HZ);

	if( setup_irq(tcttag_timer.irq, &tct_core_timer_irqaction) )
		panic("could not attach timer interrupt!");

	tct_pic_irq_unmask(tcttag_timer.irq);
}

static unsigned long get_time_offset(void)
{
	return (tct_core_timer_reg->Period - tct_core_timer_reg->Snapshot) / (CONFIG_CPU_CLOCK / HZ);
}

cycles_t get_cycles(void)
{
	if( likely(tct_core_timer_reg) )
		return tct_cycles +
			tct_core_timer_reg->Period - tct_core_timer_reg->Snapshot;
	return 0;
}

/*
 * This version of gettimeofday has near microsecond resolution.
 */
void do_gettimeofday(struct timeval *tv)
{
	unsigned long flags;
	unsigned long seq;
	unsigned long usec, sec;

	do {
		seq = read_seqbegin_irqsave(&xtime_lock, flags);
		usec = get_time_offset();
		sec = xtime.tv_sec;
		usec += (xtime.tv_nsec / 1000);
	} while (read_seqretry_irqrestore(&xtime_lock, seq, flags));

	while (usec >= 1000000) {
		usec -= 1000000;
		sec++;
	}

	tv->tv_sec = sec;
	tv->tv_usec = usec;
}
EXPORT_SYMBOL(do_gettimeofday);

int do_settimeofday(struct timespec *tv)
{
	time_t wtm_sec, sec = tv->tv_sec;
	long wtm_nsec, nsec = tv->tv_nsec;

	if ((unsigned long)tv->tv_nsec >= NSEC_PER_SEC)
		return -EINVAL;

	write_seqlock_irq(&xtime_lock);
	/*
	 * This is revolting. We need to set the xtime.tv_usec
	 * correctly. However, the value in this location is
	 * is value at the last tick.
	 * Discover what correction gettimeofday
	 * would have done, and then undo it!
	 */
	nsec -= (get_time_offset() * 1000);

	wtm_sec  = wall_to_monotonic.tv_sec + (xtime.tv_sec - sec);
	wtm_nsec = wall_to_monotonic.tv_nsec + (xtime.tv_nsec - nsec);

	set_normalized_timespec(&xtime, sec, nsec);
	set_normalized_timespec(&wall_to_monotonic, wtm_sec, wtm_nsec);

	ntp_clear();
	write_sequnlock_irq(&xtime_lock);
	clock_was_set();

	return 0;
}
EXPORT_SYMBOL(do_settimeofday);
