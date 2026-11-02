#!/usr/bin/env bash
# Build static zstd for MinGW-w64.
#   ./win32/build-zstd.sh
set -euo pipefail

cd "$(dirname "$0")/third_party"
VERSION=1.5.7
CC=${CC:-x86_64-w64-mingw32-gcc}
AR=${AR:-x86_64-w64-mingw32-ar}

if [ ! -d "zstd-$VERSION" ]; then
    if [ ! -f "zstd-$VERSION.tar.gz" ]; then
        curl -sSL -o "zstd-$VERSION.tar.gz" \
            "https://codeload.github.com/facebook/zstd/tar.gz/refs/tags/v$VERSION"
    fi
    tar xzf "zstd-$VERSION.tar.gz"
fi

make -C "zstd-$VERSION/lib" CC="$CC" AR="$AR" libzstd.a
mkdir -p zstd/lib zstd/include
cp "zstd-$VERSION/lib/libzstd.a" zstd/lib/
cp "zstd-$VERSION/lib/zstd.h" "zstd-$VERSION/lib/zstd_errors.h" \
   "zstd-$VERSION/lib/zdict.h" zstd/include/
echo "done: win32/third_party/zstd"
