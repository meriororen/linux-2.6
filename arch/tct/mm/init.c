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

#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/ptrace.h>
#include <linux/mman.h>
#include <linux/initrd.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/init.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>
#include <linux/bootmem.h>
#include <linux/slab.h>
#include <linux/linkage.h>
#include <linux/pfn.h>
#include <linux/memblock.h>

#include <asm-generic/sections.h>
#include <asm/setup.h>

unsigned long memory_start;
unsigned long memory_end;

void __init bootmem_init(void)
{
	struct memblock_region *reg;
	unsigned long bootmap_size;
	unsigned long free_pfn, end_pfn, start_pfn; /* pfn is page frame number */

	for_each_memblock(memory, reg) {
		memory_start = reg->base;
		memory_end = reg->base + reg->size;
		break;
	}

	if(((unsigned long)__pa(_end) < memory_start) || ((unsigned long)__pa(_end) > memory_end))
		printk("BUG: your kernel is not located in the ddr sdram"); /* __pa(addr) is addr - PAGE_OFFSET */

	init_mm.start_code = (unsigned long)_stext;
	init_mm.end_code = (unsigned long)_etext;
	init_mm.end_data = (unsigned long)_edata;
	init_mm.brk = (unsigned long)_end;

	start_pfn = PFN_UP(memory_start);
	free_pfn = PFN_UP(__pa((unsigned long)_end));
	end_pfn = PFN_DOWN(memory_end);

	memblock_reserve(PFN_PHYS(start_pfn), PFN_PHYS(free_pfn - start_pfn));
	
	bootmap_size = init_bootmem(free_pfn, end_pfn);
	memblock_reserve(PFN_PHYS(free_pfn), bootmap_size);

	free_bootmem(PFN_PHYS(free_pfn), PFN_PHYS(end_pfn - free_pfn);

	for_each_memblock(reserved, reg)
		reserve_bootmem(reg->base, reg->size, BOOTMEM_DEFAULT);

	memory_start += PAGE_OFFSET;
	memory_end += PAGE_OFFSET;	
}

void __init paging_init(void)
{
	unsigned long zones_size[MAX_NR_ZONES] = {0, };

	zones_size[ZONE_NORMAL] = max_low_pfn;
	free_area_init(zones_size);
}










