#ifndef ASM_TCT_SECTIONS_H
#define ASM_TCT_SECTIONS_H

#include <asm-generic/sections.h>

#ifndef __ASSEMBLY__
extern char __exception_text_start[], __exception_text_end[];
extern char __dtb_start[], __dtb_end[];
#endif

#endif
