CC=gcc
MPICC=mpicc
CILKCC=/usr/local/OpenCilk-9.0.1-Linux/bin/clang++
CFLAGS=-O3

BIN_DIR=bin
SRC_DIR=src
$(info $(shell mkdir -p $(BIN_DIR)))

default: all

V1:
	$(CC) $(CFLAGS) -o $(BIN_DIR)/v1 $(SRC_DIR)/V1.cpp

V2:
	$(CC) $(CFLAGS) -o $(BIN_DIR)/v2 $(SRC_DIR)/V2.cpp

V3_cilk:
	$(CC) $(CFLAGS) -o $(BIN_DIR)/V3_cilk $(SRC_DIR)/V3_cilk.c -fcilkplus -lcilkrts

V3_openmp:
	$(CC) $(CFLAGS) -o $(BIN_DIR)/V3_openmp $(SRC_DIR)/V3_openmp.c -fopenmp

test:
	$(CC) $(CFLAGS) -o $(BIN_DIR)/temp $(SRC_DIR)/c3.c


.PHONY: clean

all: V3_cilk V3_openmp

clean:
	rm -rf $(BIN_DIR)