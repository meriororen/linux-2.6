/*
 * Partially based on
 *
 * linux/arch/m68knommu/kernel/setup.c
 * and 
 * linux/arch/lm32/kernel/setup.c
 */

/*
 * This file handles the architecture-dependent parts of system setup
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/console.h>
#include <linux/string.h>
#include <linux/major.h>
#include <linux/initrd.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>
#include <linux/bootmem.h>
#include <linux/memblock.h>

#include <asm/sections.h>
#include <asm/pgtable.h>
#include <asm/page.h>

unsigned int kernel_mode = PT_MODE_KERNEL; /* unclear http://bit.ly/sF2mb7 */

static char __initdata cmd_line[COMMAND_LINE_SIZE];

/* we need printk */
extern void setup_early_printk(void);
/* from mm/init.c */
extern void bootmem_init(void);

unsigned int cpu_frequency;

int __init early_init_dt_scan_memory_arch(unsigned long node,
					  const char *uname, int depth,
					  void *data)
{
	return early_init_dt_scan_memory(node, uname, depth, data);
}

void __init early_init_dt_add_memory_arch(u64 base, u64 size)
{
	memblock_add(base, size);
}

void * __init early_init_dt_alloc_memory_arch(u64 size, u64 align)
{
	// return __va(memblock_alloc(size, align));
	return __alloc_bootmem(size, align, 0x0);
}

#ifdef CONFIG_BLK_DEV_INITRD
void __init early_init_dt_setup_initrd_arch(unsigned long start, unsigned long end)
{
	initrd_start = (unsigned long)__va(start);
	initrd_end = (unsigned long)__va(end);
	initrd_below_start_ok = 1;
}
#endif

void __init early_init_devtree(void *params)
{
	/* Setup flat device-tree pointer */
	initial_boot_params = params;

	/* Retreive various info from the chosen node of the
	 * device tree, including the platform type, initrd location and
  	 * size, and more ...
     	 */
	of_scan_flat_dt(early_init_dt_scan_chosen, cmd_line);

	/* Scan memory nodes */
	memblock_init();
	of_scan_flat_dt(early_init_dt_scan_root, NULL);
	of_scan_flat_dt(early_init_dt_scan_memory_arch, NULL);
	memblock_analyze();
}

static void __init device_tree_init(void)
{
	unsigned long base, size;
	struct device_node *cpu;
	int ret;

	if(!initial_boot_params) return;

	base = __pa(initial_boot_params);
	size = be32_to_cpu(initial_boot_params->totalsize);

	/* Reserve the dt blob */
	memblock_reserve(base, size);

	printk("unflatten_device_tree()...\n");
	unflatten_device_tree();

	/* free the space reserved for the dt blob */
	memblock_free(base, size);

	cpu = of_find_compatible_node(NULL, NULL, "tct");
	if(!cpu)
		panic("No compatible CPU found in device tree\n");

	ret = of_property_read_u32(cpu, "clock-frequency", &cpu_frequency);
	if(ret)
		cpu_frequency = (unsigned long)CONFIG_CPU_CLOCK;

	of_node_put(cpu);
	printk("%s: finished.\n", __func__);
}

extern char __dtb_start[];

void __init machine_early_init(char *cmdline, unsigned long p_initrd_start,
		unsigned long p_initrd_end)
{
	/* clear bss section */
	memset(__bss_start, 0, __bss_stop - __bss_start);

#ifndef CONFIG_CMDLINE_BOOL
	if (cmdline) {
		strlcpy(cmd_line, cmdline, COMMAND_LINE_SIZE);
	}
#else
	strlcpy(cmd_line, CONFIG_CMDLINE, COMMAND_LINE_SIZE);
#endif
	initrd_start = p_initrd_start;
	initrd_end = p_initrd_end;

	early_init_devtree(__dtb_start);
	printk("initrd: %lx %lx size: %lu \n", initrd_start, initrd_end, initrd_end - initrd_start);
	memblock_reserve(__pa(initrd_start), initrd_end - initrd_start);
}

void __init setup_arch(char **cmdline_p)
{
	/*
	 * init "current thread structure" pointer
 	 */
	tct_current_thread = (struct thread_info*)&init_thread_union;
	

	strlcpy(boot_command_line, cmd_line, COMMAND_LINE_SIZE);
	*cmdline_p = cmd_line;

#ifdef CONFIG_DUMMY_CONSOLE
	conswitchp = &dummy_con;
#endif
	
#ifdef CONFIG_EARLY_PRINTK
	setup_early_printk();
#endif

	/*
	 * init boot memory
	 */
	bootmem_init();
	device_tree_init();

	/*
	 * get kmalloc into gear.
	 */
	paging_init();

}

static int __init tct_device_probe(void)
{
	of_platform_populate(NULL, NULL, NULL);
	return 0;
}
arch_initcall(tct_device_probe);
