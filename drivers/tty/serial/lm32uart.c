/*
 * (C) Copyright 2009 Sebastien Bourdeauducq, Takeshi Matsuya
 * (C) Copyright 2007 Theobroma Systems <www.theobroma-systems.com>
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

#include <linux/module.h>
#include <linux/tty.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/serial.h>
#include <linux/serialP.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/hw/tct-board.h>

#define UART_DRIVERNAME "lm32_uart"
#define UART_DEVICENAME "ttyS"
#define UART_MAJOR TTY_MAJOR
#define UART_MINOR 64

static volatile int tx_cts;
static bool lm32_uart_irqs_enabled;

/* these two will be initialized by lm32uart_init */
static struct uart_port lm32uart_ports[1];

static struct uart_port* __devinit lm32uart_init_port(void);

static unsigned int lm32uart_tx_empty(struct uart_port *port);
static void lm32uart_set_mctrl(struct uart_port *port, unsigned int mctrl);
static unsigned int lm32uart_get_mctrl(struct uart_port *port);
static void lm32uart_start_tx(struct uart_port *port);
static void lm32uart_stop_tx(struct uart_port *port);
static void lm32uart_stop_rx(struct uart_port *port);
static void lm32uart_enable_ms(struct uart_port *port);
static void lm32uart_break_ctl(struct uart_port *port, int break_state);
static int lm32uart_startup(struct uart_port *port);
static void lm32uart_shutdown(struct uart_port *port);
static void lm32uart_set_termios(struct uart_port *port, struct ktermios *termios, struct ktermios *old);
static const char *lm32uart_type(struct uart_port *port);
static void lm32uart_release_port(struct uart_port *port);
static int lm32uart_request_port(struct uart_port *port);
static void lm32uart_config_port(struct uart_port *port, int flags);
static int lm32uart_verify_port(struct uart_port *port, struct serial_struct *ser);

static inline void lm32uart_set_baud_rate(struct uart_port *port, unsigned long baud);
static irqreturn_t lm32uart_irq_rx(int irq, void* portarg);
static irqreturn_t lm32uart_irq_tx(int irq, void* portarg);

static struct uart_ops lm32uart_pops = {
	.tx_empty	= lm32uart_tx_empty,
	.set_mctrl	= lm32uart_set_mctrl,
	.get_mctrl	= lm32uart_get_mctrl,
	.stop_tx	= lm32uart_stop_tx,
	.start_tx	= lm32uart_start_tx,
	.stop_rx	= lm32uart_stop_rx,
	.enable_ms	= lm32uart_enable_ms,
	.break_ctl	= lm32uart_break_ctl,
	.startup	= lm32uart_startup,
	.shutdown	= lm32uart_shutdown,
	.set_termios	= lm32uart_set_termios,
	.type		= lm32uart_type,
	.release_port	= lm32uart_release_port,
	.request_port	= lm32uart_request_port,
	.config_port	= lm32uart_config_port,
	.verify_port	= lm32uart_verify_port
};

static inline void lm32uart_set_baud_rate(struct uart_port *port, unsigned long baud)
{
	// TODO: use the board configuration option to get the frequency
	iowrite32((unsigned long)CONFIG_CPU_CLOCK/baud/16, CSR_UART_DIVISOR);
}

static void lm32uart_tx_next_char(struct uart_port* port)
{
	struct circ_buf *xmit = &(port->state->xmit);

	if (port->x_char) {
		/* send xon/xoff character */
		tx_cts = 0;
		iowrite32(port->x_char, UART_RXTX);
		port->x_char = 0;
		port->icount.tx++;
		return;
	}

	/* stop transmitting if buffer empty */
	if (uart_circ_empty(xmit) || uart_tx_stopped(port)) {
		tx_cts = 1;
		return;
	}

	/* send next character */
	tx_cts = 0;
	iowrite32(xmit->buf[xmit->tail], UART_RXTX);
	xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
	port->icount.tx++;

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);
#if 0
while(!(lm32_irq_pending() & (1 << IRQ_UART)));
#endif
}

static void lm32uart_rx_next_char(struct uart_port* port)
{
	struct tty_struct *tty = port->state->port.tty;
	unsigned char ch;

	ch = ioread32be(UART_RXTX) & 0xFF;
	port->icount.rx++;

	if (uart_handle_sysrq_char(port, ch))
		goto ignore_char;

	tty_insert_flip_char(tty, ch, TTY_NORMAL);

ignore_char:
	tty_flip_buffer_push(tty);
}

static irqreturn_t lm32uart_irq_rx(int irq, void* portarg)
{
	struct uart_port* port = (struct uart_port*)portarg;

	lm32uart_rx_next_char(port);
	
	return IRQ_HANDLED;
}

static irqreturn_t lm32uart_irq_tx(int irq, void* portarg)
{
	struct uart_port* port = (struct uart_port*)portarg;

	lm32uart_tx_next_char(port);

	return IRQ_HANDLED;
}

static unsigned int lm32uart_tx_empty(struct uart_port *port)
{
	return TIOCSER_TEMT;
}

static void lm32uart_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	/* no modem control */
}

static unsigned int lm32uart_get_mctrl(struct uart_port *port)
{
	return 0;
}

static void lm32uart_start_tx(struct uart_port *port)
{
	if (tx_cts) {
		struct circ_buf *xmit = &(port->state->xmit);

		if (port->x_char) {
			/* send xon/xoff character */
			tx_cts = 0;
			iowrite32(port->x_char, UART_RXTX);
			port->x_char = 0;
			port->icount.tx++;
			return;
		}

		/* stop transmitting if buffer empty */
		if (uart_circ_empty(xmit) || uart_tx_stopped(port))
			return;

		/* send next character */
		tx_cts = 0;
		iowrite32(xmit->buf[xmit->tail], UART_RXTX);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		port->icount.tx++;

		if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
			uart_write_wakeup(port);
	}
	return;
}

static void lm32uart_stop_tx(struct uart_port *port)
{
	return;
}


static void lm32uart_stop_rx(struct uart_port *port)
{
	return;
}

static void lm32uart_enable_ms(struct uart_port *port)
{
}

static void lm32uart_break_ctl(struct uart_port *port, int break_state)
{
}

static int lm32uart_startup(struct uart_port *port)
{
	if( request_irq(IRQ_UARTRX, lm32uart_irq_rx,
				IRQF_DISABLED, "lm32_uart RX", port) ) {
		printk(KERN_NOTICE "Unable to attach LM32 UART RX interrupt\n");
		return -EBUSY;
	}
	if( request_irq(IRQ_UART, lm32uart_irq_tx,
				IRQF_DISABLED, "lm32_uart TX", port) ) {
		printk(KERN_NOTICE "Unable to attach LM32 UART TX interrupt\n");
		return -EBUSY;
	}

	lm32_uart_irqs_enabled = true;

	return 0;
}

static void lm32uart_shutdown(struct uart_port *port)
{
	lm32_uart_irqs_enabled = false;

	free_irq(IRQ_UARTRX, port);
	free_irq(IRQ_UART, port);
}

static void lm32uart_set_termios(
		struct uart_port *port, struct ktermios *termios, struct ktermios *old)
{
	unsigned long baud;
	unsigned long flags;

	/* >> 4 means / 16 */
	baud = uart_get_baud_rate(port, termios, old, 0, port->uartclk >> 4);

	/* deactivate irqs */
	spin_lock_irqsave(&port->lock, flags);

	lm32uart_set_baud_rate(port, baud);

	uart_update_timeout(port, termios->c_cflag, baud);

	/* restore irqs */
	spin_unlock_irqrestore(&port->lock, flags);
}

static const char *lm32uart_type(struct uart_port *port)
{
	/* check, to be on the safe side */
	if( port->type == PORT_UARTLITE )
		return "lm32_uart";
	else
		return "error";
}

static void lm32uart_release_port(struct uart_port *port)
{
}

static int lm32uart_request_port(struct uart_port *port)
{
	return 0;
}

/* we will only configure the port type here */
static void lm32uart_config_port(struct uart_port *port, int flags)
{
	if( flags & UART_CONFIG_TYPE ) {
		port->type = PORT_UARTLITE;
	}
}

/* we do not allow the user to configure via this method */
static int lm32uart_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	return -EINVAL;
}

#ifdef CONFIG_SERIAL_LM32_CONSOLE
static void lm32_console_putchar(struct uart_port *port, int ch)
{
	if (lm32_uart_irqs_enabled)
		disable_irq(IRQ_UART);
	iowrite32(ch, UART_RXTX);
	while(!(lm32_irq_pending() & (1 << IRQ_UART)));
	lm32_irq_ack(IRQ_UART);
	if (lm32_uart_irqs_enabled)
		enable_irq(IRQ_UART);
}

/*
 * Interrupts are disabled on entering
 */
static void lm32_console_write(struct console *co, const char *s, u_int count)
{
	struct uart_port *port = &lm32uart_ports[co->index];

	uart_console_write(port, s, count, lm32_console_putchar);
}

static int __init lm32_console_setup(struct console *co, char *options)
{
	struct uart_port *port = &lm32uart_ports[co->index];

	int baud = 115200;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);

	return uart_set_options(port, co, baud, parity, bits, flow);
}

static struct uart_driver lm32uart_driver;

static struct console lm32_console = {
	.name		= UART_DEVICENAME,
	.write		= lm32_console_write,
	.device		= uart_console_device,
	.setup		= lm32_console_setup,
	.flags		= CON_PRINTBUFFER,
	.index		= -1,
	.data		= &lm32uart_driver,
};

/*
 * Early console initialization
 */
static int __init lm32_early_console_init(void)
{
	add_preferred_console(UART_DEVICENAME, 0, NULL);
	lm32uart_init_port();
	register_console(&lm32_console);
	pr_info("lm32_uart: registered real console\n");
	return 0;
}
console_initcall(lm32_early_console_init);

/*
 * Late console initialization
 */
static int __init lm32_late_console_init(void)
{
	if( !(lm32_console.flags & CON_ENABLED) ) {
		register_console(&lm32_console);
		pr_info("lm32_uart: registered real console\n");
	}
	return 0;
}
core_initcall(lm32_late_console_init);

#define LM32_CONSOLE_DEVICE	&lm32_console
#else
#define LM32_CONSOLE_DEVICE	NULL
#endif

static struct uart_driver lm32uart_driver = {
	.owner       = THIS_MODULE,
	.driver_name = UART_DRIVERNAME,
	.dev_name    = UART_DEVICENAME,
	.major       = UART_MAJOR,
	.minor       = UART_MINOR,
	.nr          = 0, /* will be filled by init */
	.cons        = MILKYMIST_CONSOLE_DEVICE
};

static struct uart_port* __devinit lm32uart_init_port(void)
{
	struct uart_port* port;
	
	port = &lm32uart_ports[0];
	port->type = PORT_UARTLITE;
	port->iobase = 0x0;
	port->membase = (void __iomem*)UART_RXTX;
	port->irq = IRQ_UARTRX;
	port->uartclk = CONFIG_CPU_CLOCK;
	port->flags = UPF_SKIP_TEST | UPF_BOOT_AUTOCONF; // TODO perhaps this is not completely correct
	port->iotype = UPIO_PORT; // TODO perhaps this is not completely correct
	port->regshift = 0;
	port->ops = &lm32uart_pops;
	port->line = 0;
	port->private_data = NULL;
	return port;
}

static int __devinit lm32uart_serial_probe(struct platform_device *pdev)
{
	struct uart_port *port;
	int ret;

	port = lm32uart_init_port();

	ret = uart_add_one_port(&lm32uart_driver, port);
	if (!ret) {
		pr_info("lm32_uart: added port %d with irq %d-%d at 0x%lx\n",
				port->line, IRQ_UARTRX, IRQ_UART, (unsigned long)port->membase);
		device_init_wakeup(&pdev->dev, 1);
		platform_set_drvdata(pdev, port);
	} else
		printk(KERN_ERR "lm32_uart: could not add port %d: %d\n", port->line, ret);

	return ret;
}

static int __devexit lm32uart_serial_remove(struct platform_device *pdev)
{
	struct uart_port *port = platform_get_drvdata(pdev);
	int ret = 0;

	device_init_wakeup(&pdev->dev, 0);
	platform_set_drvdata(pdev, NULL);

	if (port) {
		ret = uart_remove_one_port(&lm32uart_driver, port);
		kfree(port);
	}

	return ret;
}

static const struct of_device_id lm32uart_of_ids[] = {
	{ .compatible = "lm32,uart", },
	{}
};

static struct platform_driver lm32uart_serial_driver = {
	.probe		= lm32uart_serial_probe,
	.remove		= __devexit_p(lm32uart_serial_remove),
	.driver		= {
		.name	= "lm32_uart",
		.owner	= THIS_MODULE,
		.of_match_table = lm32uart_of_ids,
	},
};

static int __init lm32uart_init(void)
{
	int ret;
	
	pr_info("lm32_uart: LM32 UART driver (modified for TCT)\n");

	/* configure from hardware setup structures */
	lm32uart_driver.nr = 1;
	tx_cts = 1;
	ret = uart_register_driver(&lm32uart_driver);
	if( ret < 0 )
		return ret;

	ret = platform_driver_register(&lm32uart_serial_driver);
	if( ret < 0 )
		uart_unregister_driver(&lm32uart_driver);

	return ret;
}

static void __exit lm32uart_exit(void)
{
	platform_driver_unregister(&lm32uart_serial_driver);
	uart_unregister_driver(&lm32uart_driver);
}

module_init(lm32uart_init);
module_exit(lm32uart_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LM32 UART driver modified for TCT");
MODULE_AUTHOR("Kunieda-Isshiki Lab Linux Project");
