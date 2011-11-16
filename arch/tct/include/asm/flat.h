/*
 * uClinux flat-format executables
 *
 * 
 *
 */

#ifndef __ASM_TCT_FLAT_H
#define __ASM_TCT_FLAT_H

#include <asm/unaligned.h>

#define flat_argvp_envp_on_stack()		0
#define flat_old_ram_flag(flags)		(flags)

extern unsigned long tct_get_addr_from_rp (unsigned long *ptr,
					unsigned long relval,
					unsigned long flags,
					unsigned long *persistent);

extern void tct_put_addr_at_rp (unsigned long *ptr, unsigned long addr,
				unsigned long relval);

/* The amount by which a relocation can exceed the program image limits
   without being regarded as error. */

#define flat_reloc_valid(reloc, size)	((reloc) <= (size))

#define flat_get_addr_from_rp(rp, relval, flags, persistent)	\
	tct_get_addr_from_rp(rp, relval, flags, persistent)
#define flat_put_addr_at_rp(rp, val, relval) 	\
	tct_put_addr_at_rp(rp, val, relval)

/* Convert a relocation entry into an address. */
static inline unsigned long
flat_get_relocate_addr (unsigned long relval)
{
	return relval & 0x1fffffff; /* Mask out top 3 bits */
}

static inline int flat_set_persistent (unsigned long relval,
					unsigned long *persistent)
{
	int type = (relval >> 29) & 7;
	if (type == 3) {
		*persistent = relval << 16;
		return 1;
	}
	return 0;
}

#endif
