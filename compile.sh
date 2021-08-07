#!/bin/sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
cc main.c -g -std=c99 -c -I /opt/raylib/examples/models/ -o build/main.o &&
cc build/main.o -s -Wall -std=c99 -I/opt/raylib/src -L/usr/local/lib/ -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o build/cginc &&
./build/cginc test.nc resources/test.stl
