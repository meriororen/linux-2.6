#include <linux/moduleloader.h>
#include <linux/elf.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/kernel.h>

#if 0
 #define DEBUGP printk
#else
 #define DEBUGP(fmt...)
#endif

void *module_alloc(unsigned long size)
{
	if(size == 0)
		return NULL;
	return vmalloc(size);
}

/* Free memory returned from module_alloc */
void module_free(struct module *mod, void *module_region)
{
	vfree(module_region);
}

int module_fron_arch_sections(Elf_Ehdr *hdr, Elf_Shdr *sechdrs,
			      char *secstrings, struct module *mod)
{ 
	return 0;
}

static 
int relocate_at(int rtype, unsigned* insn_addr, unsigned target_addr);

int apply_relocate(Elf32_Shdr *sechdrs,
		   const char *strtab,
		   unsigned int symindex,
		   unsigned int relsec,
		   struct module *me)
{
	unsigned int i;
	Elf32_Rel *rel = (void *)sechdrs[relsec].sh_addr;
	Elf32_Sym *sym;
	uint32_t *location;

	DEBUGP("Applying relocate section %u to %u\n", relsec,
		sechdrs[relsec].sh_info);
	for(i = 0; i < sechdrs[relsec].sh_size / sizeof(*rel); i++) {
		/* This is where to make the change */
		location = (void *)sechdrs[sechdrs[relsec].sh_info].sh_addr
			 + rel[i].r_offset;
		/* This is the symbol it is referring to. Note that all
		   undefined symbols have been resolved. */
		sym = (Elf32_Sym *)sechdrs[symindex].sh_addr
			 + ELF32_R_SYM(rel[i].r_info);
		
		if( -1 == relocate_at(
					ELF32_R_TYPE(rel[i].r_info),
					location,
					sym->st_value) )
			return -ENOEXEC;
	}
	return 0;
}

int apply_relocate_add(Elf32_Shdr *sechdrs,
		       const char *strtab,
		       unsigned int symindex,
		       unsigned int relsec,
		       struct module *me)
{
	
}
