#!/usr/bin/env bash
# fetch-node.sh — resolve the Node version a SEA was built against, fetch the
# matching musl runtime (cached in DL_DIR), and stage its node binary as <out>.
#
# Usage:
#   fetch-node.sh <glibc-sea-binary> <node-arch> <dl-dir> <out-binary> [fallback-version]
#     node-arch: x64 | arm64
#
# The version is dynamic, so no hash is pinned in the Makefile. Instead the
# cached/downloaded tarball is verified against the per-version SHASUMS256.txt
# (catches corruption/truncation — integrity, not GPG authenticity). Download is
# atomic: a half-written file never poisons the cache.
set -euo pipefail

GLIBC_BIN="$1"; NODE_ARCH="$2"; DL_DIR="$3"; OUT="$4"; FALLBACK="${5:-}"
BASE="https://unofficial-builds.nodejs.org/download/release"

die()     { echo "fetch-node: $*" >&2; exit 1; }
tarball() { echo "node-v$1-linux-${NODE_ARCH}-musl.tar.xz"; }
url()     { echo "$BASE/v$1/$(tarball "$1")"; }
exists()  { curl -fsI "$(url "$1")" >/dev/null 2>&1; }

# 1. detect the version baked into the SEA (process.release URL)
ver="$(strings "$GLIBC_BIN" \
  | grep -oE 'nodejs\.org/download/release/v[0-9]+\.[0-9]+\.[0-9]+' \
  | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | sort -uV | tail -n1)"
[ -n "$ver" ] || die "could not detect node version in $GLIBC_BIN"
echo "fetch-node: SEA built against node v$ver"

# 2. exact version, else the configured fallback (keep it on the same major)
if ! exists "$ver"; then
	echo "fetch-node: no musl build for v$ver"
	[ -n "$FALLBACK" ]   || die "no musl build for v$ver and no fallback set"
	exists "$FALLBACK"   || die "fallback v$FALLBACK has no musl build either"
	echo "fetch-node: falling back to v$FALLBACK"
	ver="$FALLBACK"
fi

file="$(tarball "$ver")"
path="$DL_DIR/$file"
mkdir -p "$DL_DIR"

# 3. expected checksum from upstream (best-effort)
expected="$(curl -fsSL "$BASE/v$ver/SHASUMS256.txt" 2>/dev/null \
  | awk -v f="$file" '$2==f {print $1}' || true)"
[ -n "$expected" ] || echo "fetch-node: WARNING: no upstream checksum, integrity unchecked" >&2
verify() { [ -z "$expected" ] || echo "$expected  $1" | sha256sum -c - >/dev/null 2>&1; }

# 4. reuse the cached tarball if valid, else download atomically
if [ -f "$path" ] && verify "$path"; then
	echo "fetch-node: using cached $file"
else
	[ -f "$path" ] && echo "fetch-node: cached $file failed check, refetching"
	echo "fetch-node: downloading $file"
	tmp="$path.part.$$"
	trap 'rm -f "$tmp"' EXIT
	curl -fSL "$(url "$ver")" -o "$tmp"
	verify "$tmp" || die "checksum mismatch on freshly downloaded $file"
	mv -f "$tmp" "$path"
	trap - EXIT
fi

# 5. stage just the node binary -> OUT
echo "fetch-node: staging node binary -> $OUT"
tar -xJf "$path" -C "$(dirname "$OUT")" --strip-components=2 \
	"node-v${ver}-linux-${NODE_ARCH}-musl/bin/node"
mv -f "$(dirname "$OUT")/node" "$OUT"
chmod 0755 "$OUT"
echo "fetch-node: done (node v$ver)"
