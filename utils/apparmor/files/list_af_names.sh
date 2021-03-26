#!/bin/bash -e

# =====================
# generate list of network protocols based on
# sys/socket.h for use in multiple locations in
# the source tree
# =====================

# It doesn't make sence for AppArmor to mediate PF_UNIX, filter it out. Search
# for "PF_" constants since that is what is required in bits/socket.h, but
# rewrite as "AF_".

echo "#include <sys/socket.h>" | \
  cpp -dM | \
  LC_ALL=C sed -n \
    -e '/PF_UNIX/d' \
    -e 's/PF_LOCAL/PF_UNIX/' \
    -e 's/^#define[ \t]\+PF_\([A-Z0-9_]\+\)[ \t]\+\([0-9]\+\).*$/AF_\1 \2,/p' | \
  sort -n -k2
