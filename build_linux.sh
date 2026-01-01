#!/usr/bin/env bash

build="bin"
bin="sandbox"
src="src/main.c"

SYSLIBS="-lm"

rm -rf "$build"
mkdir -p "$build"

echo "$src -> $build/$bin"
zig cc "$src" -o "$build/$bin" $SYSLIBS -g -std=c11
