#!/bin/sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
RAYLIB_PATH=/opt/homebrew/Cellar/raylib/5.5
CFLAGS=$(pkg-config --cflags raylib)
CFLAG="${CFLAGS} -g -O2 -Wall -fsanitize=address -fsanitize=undefined"
LDFLAGS=$(pkg-config --libs raylib)


mkdir -p build &&
cc main.c -c ${CFLAGS} -o build/main.o &&
cc build/main.o -s ${CFLAGS} -L/usr/local/lib/ ${LDFLAGS} -o build/cginc &&
./build/cginc test.nc resources/test.stl
