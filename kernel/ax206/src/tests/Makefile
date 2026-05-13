# SPDX-License-Identifier: GPL-2.0
#
# AX206 userspace DRM test utility
# Copyright (C) 2025 Oskari Rauta <oskari.rauta@gmail.com>

CFLAGS  := -O2 -Wall -Wextra -I/usr/include/libdrm
LDFLAGS := -ldrm

all: ax206-test

ax206-test: ax206-test.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f ax206-test

.PHONY: all clean
