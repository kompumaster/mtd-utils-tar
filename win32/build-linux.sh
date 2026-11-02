#!/usr/bin/env bash
# Native build of mkfs.ubifs with --tar (same patch as the Windows build).
#   ./win32/build-linux.sh [output]
set -euo pipefail

cd "$(dirname "$0")/.."
CC=${CC:-gcc}
OUT=${1:-win32/mkfs.ubifs_tar}
OBJ=win32/obj-linux
UUID_INC=$(pkg-config --variable=includedir uuid 2>/dev/null || true)

CFLAGS=(
    -O2 -g -std=gnu11
    -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE
    -DWITH_ZLIB -DWITH_LZO -DWITH_ZSTD
    -Iinclude -Iubifs-utils -Iubifs-utils/common -Iubifs-utils/libubifs
    -Iubifs-utils/mkfs.ubifs
    ${UUID_INC:+-I$UUID_INC/uuid}
    -include win32/compat/linux_build.h
)

COMMON=(
    ubifs-utils/common/bitops.c
    ubifs-utils/common/kmem.c
    ubifs-utils/common/sort.c
    ubifs-utils/common/crc16.c
    ubifs-utils/common/compr.c
    ubifs-utils/common/devtable.c
    ubifs-utils/common/hexdump.c
    ubifs-utils/common/hashtable/hashtable.c
    ubifs-utils/common/hashtable/hashtable_itr.c
)

LIBUBIFS=(
    ubifs-utils/libubifs/io.c ubifs-utils/libubifs/sb.c
    ubifs-utils/libubifs/super.c ubifs-utils/libubifs/master.c
    ubifs-utils/libubifs/debug.c ubifs-utils/libubifs/scan.c
    ubifs-utils/libubifs/find.c ubifs-utils/libubifs/dir.c
    ubifs-utils/libubifs/budget.c ubifs-utils/libubifs/journal.c
    ubifs-utils/libubifs/gc.c ubifs-utils/libubifs/lpt.c
    ubifs-utils/libubifs/lpt_commit.c ubifs-utils/libubifs/lprops.c
    ubifs-utils/libubifs/tnc_misc.c ubifs-utils/libubifs/tnc.c
    ubifs-utils/libubifs/tnc_commit.c ubifs-utils/libubifs/orphan.c
    ubifs-utils/libubifs/recovery.c ubifs-utils/libubifs/replay.c
    ubifs-utils/libubifs/commit.c ubifs-utils/libubifs/log.c
)

SOURCES=(
    "${COMMON[@]}"
    "${LIBUBIFS[@]}"
    ubifs-utils/mkfs.ubifs/mkfs.ubifs.c
    lib/common.c
    lib/libcrc32.c
    lib/rbtree.c
    lib/list_sort.c
    lib/execinfo.c
    win32/compat/ubi_stub.c
    ubifs-utils/mkfs.ubifs/tar_index.c
)

mkdir -p "$OBJ"
objects=()
for src in "${SOURCES[@]}"; do
    obj="$OBJ/$(echo "$src" | tr '/' '_' | sed 's/\.c$/.o/')"
    "$CC" "${CFLAGS[@]}" -c -o "$obj" "$src"
    objects+=("$obj")
done

"$CC" -o "$OUT" "${objects[@]}" -lz -llzo2 -lzstd -luuid -lm
echo "done: $OUT"
