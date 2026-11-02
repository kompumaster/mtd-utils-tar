#!/usr/bin/env bash
# Build static zlib for MinGW-w64.
#   ./win32/build-zlib.sh
set -euo pipefail

cd "$(dirname "$0")/third_party"
VERSION=1.3.1
CC=${CC:-x86_64-w64-mingw32-gcc}
AR=${AR:-x86_64-w64-mingw32-ar}
RC=${RC:-x86_64-w64-mingw32-windres}

if [ ! -d "zlib-$VERSION" ]; then
    if [ ! -f "zlib-$VERSION.tar.gz" ]; then
        curl -sSL -o "zlib-$VERSION.tar.gz" \
            "https://codeload.github.com/madler/zlib/tar.gz/refs/tags/v$VERSION"
    fi
    tar xzf "zlib-$VERSION.tar.gz"
fi

make -C "zlib-$VERSION" -f win32/Makefile.gcc \
    CC="$CC" AR="$AR" RC="$RC" libz.a
mkdir -p zlib/lib zlib/include
cp "zlib-$VERSION/libz.a" zlib/lib/
cp "zlib-$VERSION/zlib.h" "zlib-$VERSION/zconf.h" zlib/include/
echo "done: win32/third_party/zlib"
