#!/usr/bin/env python3
# Carve the NODE_SEA_BLOB note descriptor out of a Node.js SEA ELF.
# Pure stdlib — avoids LIEF's note-walker choking on the large SEA note.
import struct, sys

def main():
    src, dst = sys.argv[1], sys.argv[2]
    with open(src, "rb") as f:
        buf = f.read()
    NAME = b"NODE_SEA_BLOB\x00"
    best = None
    i = buf.find(NAME)
    while i != -1:
        hdr = i - 12                                  # ELF note header: namesz, descsz, type
        if hdr >= 0:
            namesz, descsz, _ = struct.unpack_from("<III", buf, hdr)
            desc = i + ((namesz + 3) & ~3)            # name is padded to 4 bytes
            if namesz == len(NAME) and 0 < descsz <= len(buf) - desc:
                if best is None or descsz > best[0]:  # the real blob is the largest hit
                    best = (descsz, desc)
        i = buf.find(NAME, i + 1)
    if best is None:
        sys.exit("NODE_SEA_BLOB note not found")
    descsz, desc = best
    with open(dst, "wb") as out:
        out.write(buf[desc:desc + descsz])
    print(f"extracted {descsz} bytes -> {dst}")

if __name__ == "__main__":
    main()
