#!/usr/bin/env bash
set -e

build="bin"
bin="c_starter"
src="src/main.c"

mode="${1:-debug}"

rm -rf "$build"
mkdir -p "$build"

CFLAGS="-std=c11"

case "$mode" in
    debug)
        CFLAGS="$CFLAGS -O0 -g"
        ;;
    release)
        CFLAGS="$CFLAGS -O2 -DNDEBUG"
        ;;
    *)
        echo "unknown build mode: $mode"
        echo "usage: ./build.sh <debug|release>"
        exit 1
        ;;
esac

echo "compiling ($src) -> $build/$bin [$mode]"
clang $src -o $build/$bin $CFLAGS
