# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2023 Luca Barbato and Donald Hoskins

# Rust Environmental Vars
CONFIG_HOST_SUFFIX:=$(shell cut -d"-" -f4 <<<"$(GNU_HOST_NAME)")
RUSTC_HOST_ARCH:=$(HOST_ARCH)-unknown-linux-$(CONFIG_HOST_SUFFIX)
CARGO_HOME:=$(STAGING_DIR_HOST)/cargo

# Support only a subset for now.
RUST_ARCH_DEPENDS:=@(aarch64||arm||i686||mips||mipsel||mips64||mips64el||mipsel||powerpc64||x86_64)

# Common Build Flags
RUST_BUILD_FLAGS = \
  CARGO_HOME="$(CARGO_HOME)"

# This adds the rust environmental variables to Make calls
MAKE_FLAGS += $(RUST_BUILD_FLAGS)

# mips64 openwrt has a specific targed in rustc
ifeq ($(ARCH),"mips64")
  RUSTC_TARGET_ARCH:=$(REAL_GNU_TARGET_NAME)
else
  RUSTC_TARGET_ARCH:=$(subst openwrt,unknown,$(REAL_GNU_TARGET_NAME))
endif

RUSTC_TARGET_ARCH:=$(subst muslgnueabi,musleabi,$(RUSTC_TARGET_ARCH))

# ARM Logic
ifeq ($(ARCH),"arm")
  ifeq ($(CONFIG_arm_v7),y)
    RUSTC_TARGET_ARCH:=$(subst arm,armv7,$(RUSTC_TARGET_ARCH))
  endif

  ifeq ($(CONFIG_HAS_FPU),y)
    RUSTC_TARGET_ARCH:=$(subst musleabi,musleabihf,$(RUSTC_TARGET_ARCH))
  endif
endif

ifeq ($(ARCH),aarch64)
    RUST_CFLAGS:=-mno-outline-atomics
endif
