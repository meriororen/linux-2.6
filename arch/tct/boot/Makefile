obj-y += tct-board.dtb.o

clean-files := *.dtb.S

$(obj)/%.dtb: $(src)/dts/%.dts
	$(call cmd,dtc)

