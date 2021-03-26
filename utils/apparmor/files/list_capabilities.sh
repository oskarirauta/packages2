#!/bin/bash -e

# =====================
# generate list of capabilities based on
# /usr/include/linux/capabilities.h for use in multiple locations in
# the source tree
# =====================

echo "#include <linux/capability.h>" | \
  cpp -dM | \
  LC_ALL=C sed -n \
    -e '/CAP_EMPTY_SET/d' \
    -e 's/^\#define[ \t]\+CAP_\([A-Z0-9_]\+\)[ \t]\+\([0-9xa-f]\+\)\(.*\)$/CAP_\1/p' | \
  LC_ALL=C sort
