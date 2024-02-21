obj-m += cpuload.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement -Wno-unused-result
ifeq ($(MAKING_MODULES),1)
-include $(TOPDIR)/Rules.make
endif
