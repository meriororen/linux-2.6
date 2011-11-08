board-$(CONFIG_BOARD_TCT_BOARD)	:= tct-board
BOARD := $(board-y)

export BOARD

cflags-y := -mmultiply-enabled -mdivide-enabled \
	 -mbarrel-shift-enabled -msign-extend-enabled

KBUILD_AFLAGS += $(cflags-y)
KBUILD_CFLAGS += $(cflags-y)

OBJCOPYFLAGS := -O binary -R .note -R .comment -S

head-y := arch/tct/kernel/head.o arch/tct/kernel/init_task.o

core-y := arch/tct/
core-y := arch/tct/platforms/
libs-y := arch/tct/lib/

# some note :
# $@ means target, $< means first prerequisite
# refer to "GNU make auto variable"

vmlinux.bin: vmlinux
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@
vmlinux.gz:
	cat $< |gzip >$@
