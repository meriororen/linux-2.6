#include <linux/module.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/sem.h>
#include <linux/msg.h>
#include <linux/shm.h>
#include <linux/stat.h>
#include <linux/syscalls.h>
#include <linux/mman.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/utsname.h>

#include <asm/uaccess.h>
#include <asm/unistd.h>

asmlinkage long
sys_mmap2(unsigned long addr, unsigned long
