#ifndef _ASM_TCT_REGISTERS_H
#define _ASM_TCT_REGISTERS_H

/* special register masks */
#define MSR_USER	(0x1	<< 0) /* user mode			*/
#define MSR_MMU		(0x1	<< 1) /* MMU enable			*/
#define MSR_SUSER	(0x1	<< 2) /* saved USER bit		*/
#define MSR_SMMU	(0x1	<< 3) /* saved MMU bit		*/
#define MSR_EXCP	(0x1	<< 4) /* exception enable	*/
#define MSR_IRQ		(0x1	<< 5) /* IRQ enable			*/
#define MSR_UM		(MSR_USER	| MSR_MMU)
#define MSR_SUM		(MSR_SUSER	| MSR_SMMU)
#define MSR_UMSUM	(MSR_UM		| MSR_SUM)
#define ESR_CODE	(0x1F	<< 0) /* exception code		*/
#define ESR_STAT	(0x1F	<< 5) /* exception status	*/


#endif
