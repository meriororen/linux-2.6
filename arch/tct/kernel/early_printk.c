#include <linux/console.h>
#include <linux/init.h>
#include <linux/string.h>
#include <asm/irq.h>
#include <asm/hw/tct-board.h>
#include <linux/io.h>

static void __init early_console_putc(char c)
{

	unsigned int timeout = 10;
	uint32_t pending;
	
	iowrite32(c, UART_RXTX);
	
 	do {
		pending = tct_irq_pending();
	} while ((pending & BIT(IRQ_UARTTX)) && --timeout);

	tct_irq_ack(IRQ_UARTTX);
}

static void __init early_console_write(struct console *con, char *s,
	unsigned n)
{
	while (n-- && *s) {
		early_console_putc(*s);
		s++;
	}
}

static struct console early_console __initdata = {
	.name = "early",
	.write = early_console_write,
	.flags = CON_PRINTBUFFER | CON_BOOT,
	.index = -1
};

static bool early_console_initialized __initdata;

void __init setup_early_printk(void)
{
	if (early_console_initialized) return;
	early_console_initialized = true;
	register_console(&early_console);
}
