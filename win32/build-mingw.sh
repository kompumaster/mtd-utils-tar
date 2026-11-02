#!/usr/bin/env bash
# Build mkfs.ubifs for Windows (MinGW-w64), without Cygwin or WSL.
#
#   ./win32/build-mingw.sh [output.exe]
#
# Uses the --tar patch and win32/compat.
set -euo pipefail

cd "$(dirname "$0")/.."
CC=${CC:-x86_64-w64-mingw32-gcc}
OUT=${1:-win32/mkfs.ubifs_tar.exe}
OBJ=win32/obj

CFLAGS=(
    -O2 -g -std=gnu11
    -D_FILE_OFFSET_BITS=64
    -DWITH_ZLIB
    -DWITH_LZO
    -DWITH_ZSTD
    -Iwin32/compat/include
    -Iwin32/third_party/zlib/include
    -Iwin32/third_party/lzo/include
    -Iwin32/third_party/zstd/include
    -Iubifs-utils/mkfs.ubifs
    -Iinclude -Iubifs-utils -Iubifs-utils/common -Iubifs-utils/libubifs
    -include win32/compat/compat.h
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
    ubifs-utils/libubifs/io.c
    ubifs-utils/libubifs/sb.c
    ubifs-utils/libubifs/super.c
    ubifs-utils/libubifs/master.c
    ubifs-utils/libubifs/debug.c
    ubifs-utils/libubifs/scan.c
    ubifs-utils/libubifs/find.c
    ubifs-utils/libubifs/dir.c
    ubifs-utils/libubifs/budget.c
    ubifs-utils/libubifs/journal.c
    ubifs-utils/libubifs/gc.c
    ubifs-utils/libubifs/lpt.c
    ubifs-utils/libubifs/lpt_commit.c
    ubifs-utils/libubifs/lprops.c
    ubifs-utils/libubifs/tnc_misc.c
    ubifs-utils/libubifs/tnc.c
    ubifs-utils/libubifs/tnc_commit.c
    ubifs-utils/libubifs/orphan.c
    ubifs-utils/libubifs/recovery.c
    ubifs-utils/libubifs/replay.c
    ubifs-utils/libubifs/commit.c
    ubifs-utils/libubifs/log.c
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
    win32/compat/compat.c
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

"$CC" -static -o "$OUT" "${objects[@]}" win32/third_party/zlib/lib/libz.a win32/third_party/lzo/lib/liblzo2.a win32/third_party/zstd/lib/libzstd.a
echo "done: $OUT"
