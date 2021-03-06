/*
 * Milkymist VJ SoC (Software)
 * Copyright (C) 2007, 2008, 2009 Sebastien Bourdeauducq
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __HW_UART_H
#define __HW_UART_H

#define CSR_REG(x) ((void __iomem *)(x))

#define CSR_UART_RXTX 		CSR_REG(0xe0000000)
#define CSR_UART_DIVISOR	CSR_REG(0xe0000004)
#define CSR_UART_THRU		CSR_REG(0xe0000008)

#endif /* __HW_UART_H */
