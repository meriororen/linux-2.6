#
# arch/tct/Makefile
#

board-$(CONFIG_TCT_BOARD) := tct-board
BOARD := $(board-y)

export BOARD


OBJCOPYFLAGS := -O binary -R .note -R .comment -S

head-y := arch/tct/kernel/head.o arch/tct/kernel/init_task.o

core-y += arch/tct/
core-y += arch/tct/boot/
libs-y += arch/tct/lib/

# some note :
# $@ means target, $< means first prerequisite, while $^ 
# means all prerequisites
# refer to "GNU make auto variable"
# for example, vmlinux is first (and only) prerequisite and vmlinux.bin is target

vmlinux.bin: vmlinux 
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@
vmlinux.gz: vmlinux.bin
	cat $< |gzip >$@
