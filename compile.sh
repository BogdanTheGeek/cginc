#!/bin/sh
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
CFLAGS=$(pkg-config --cflags raylib)
CFLAG="${CFLAGS} -g -O2 -std=c11 -Wall -fsanitize=address -fsanitize=undefined"

LDFLAGS=$(pkg-config --libs raylib)


mkdir -p build &&
cc main.c -c ${CFLAGS} -o build/main.o &&
cc build/main.o -s ${CFLAGS} -L/usr/local/lib/ ${LDFLAGS} -lm -o build/cginc &&
./build/cginc test.nc resources/test.stl
