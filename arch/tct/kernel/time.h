/* These structures are component of lm32 board, it should have been used
 * by setup_arch to fetch hwsetup structure inside ROM. a thing that isn't implemented
 * on TCT-qemu see scqemu/hw/lm32_boards.c
 */

typedef struct TCTTag_Timer
{
	/* Instance Name */
	char name[32];
	/* Base Address */
	u32 addr;
	/* Writeable Tick Count (Flag) */
	u8 wr_tickcount;
	/* Readable Tick Count (Flag) */
	u8 rd_tickcount;
	/* Start Stop Control (Flag) */
	u8 start_stop_control;
	/* Counter Width (Bits) */
	u8 counter_width;
	/* Default Reload Tick */
	u32 reload_ticks;
	/* IRQ */
	u8 irq;
	/* reserved and for alignment */
	u8 reserved0;
	/* reserved and for alignment */
	u8 reserved1;
	/* reserved and for alignment */
	u8 reserved2;
} TCTTag_Timer_t;

typedef struct TCT_timer {
  volatile unsigned int  Status;
  volatile unsigned int  Control;
  volatile unsigned int  Period;
  volatile unsigned int  Snapshot;
} TCT_timer_t;

/* Only use timer 0 */
TCTTag_Timer_t tcttag_timer;

tct_tagtimer.name = "LM32-timer0";
tct_tagtimer.addr = TIMER0_CONTROL;
tct_tagtimer.wr_tickcount = 1;
tct_tagtimer.rd_tickcount = 1;
tct_tagtimer.start_stop_control = 1;
tct_tagtimer.counter_width = 32;
tct_tagtimer.reload_ticks = 20;
tct_tagtimer.irq = IRQ_TIMER0;
tct_tagtimer.reserved0 = 0;
tct_tagtimer.reserved1 = 0L;
tct_tagtimer.reserved2 = 0;

