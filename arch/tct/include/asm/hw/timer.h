

#ifndef __HW_SYSCTL_H
#define __HW_SYSCTL_H

#define REG(x) ((void __iomem *)(x))

#define TIMER0_CONTROL	REG(0x10001000)
#define TIMER0_COMPARE	REG(0x10001004)
#define TIMER0_COUNTER	REG(0x10001008)

#define TIMER1_CONTROL	REG(0x10002000)
#define TIMER1_COMPARE	REG(0x10002004)
#define TIMER1_COUNTER	REG(0x10002008)

#define TIMER(id, reg) REG(0x10001000 + (0x1000 * (id)) + (reg))

#define TIMER_CONTROL(id) TIMER(id, 0x0)
#define TIMER_COMPARE(id) TIMER(id, 0x4)
#define TIMER_COUNTER(id) TIMER(id, 0x8)

#define TIMER_ENABLE		(0x01)
#define TIMER_AUTORESTART	(0x02)

#endif /* __HW_SYSCTL_H */
