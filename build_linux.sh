#!/usr/bin/env bash

build="bin"
bin="sandbox"
src="src/main.c"

rm -rf "$build"
mkdir -p "$build"

echo "Compiling $src -> $build/$bin"
clang $src -o $build/$bin -std=c11 -lm -g -O0
