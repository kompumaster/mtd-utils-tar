#!/usr/bin/env bash
# Build static lzo2 for MinGW-w64.
#   ./win32/build-lzo.sh
set -euo pipefail

cd "$(dirname "$0")/third_party"
VERSION=2.10
CC=${CC:-x86_64-w64-mingw32-gcc}

if [ ! -d "lzo-$VERSION" ]; then
    if [ ! -f "lzo-$VERSION.tar.gz" ]; then
        curl -sSL -o "lzo-$VERSION.tar.gz" \
            "https://www.oberhumer.com/opensource/lzo/download/lzo-$VERSION.tar.gz"
    fi
    tar xzf "lzo-$VERSION.tar.gz"
fi

cd "lzo-$VERSION"
if [ ! -f config.status ]; then
    ./configure --host=x86_64-w64-mingw32 --disable-shared --enable-static \
        --prefix="$PWD/../lzo"
fi
make -j"$(nproc)"
cd ..
mkdir -p lzo/lib lzo/include
cp "lzo-$VERSION/src/.libs/liblzo2.a" lzo/lib/
cp -r "lzo-$VERSION/include/lzo" lzo/include/
echo "done: win32/third_party/lzo"
