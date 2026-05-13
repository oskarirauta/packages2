# SPDX-License-Identifier: GPL-2.0
#
# AX206 digital photo frame DRM driver
# Copyright (C) 2025 Oskari Rauta <oskari.rauta@gmail.com>

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	$(MAKE) -C ./tests

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	$(MAKE) -C ./tests clean

modules_install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install

.PHONY: all clean modules_install
