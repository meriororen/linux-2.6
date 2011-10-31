#ifndef __TCT_MMU_H
#define __TCT_MMU_H

typedef struct {
	struct vm_list_struct	*vmlist;
	unsigned long 		end_brk;

#ifdef CONFIG_BINFMT_ELF_FDPIC
	unsigned long	exec_fdpic_loadmap;
	unsigned long	interp_fdpic_loadmap;
#endif
} mm_context_t;

#endif
