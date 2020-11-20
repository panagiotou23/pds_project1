CC=g++
MPICC=mpicc
CILKCC=/usr/local/OpenCilk-9.0.1-Linux/bin/clang
CFLAGS=-O3

BIN_DIR=bin
SRC_DIR=src
$(info $(shell mkdir -p $(BIN_DIR)))

default: all

V1:
	$(CC) $(CFLAGS) -o $(BIN_DIR)/v1 $(SRC_DIR)/V1.cpp

V2:
	$(CC) $(CFLAGS) -o $(BIN_DIR)/v2 $(SRC_DIR)/V2.cpp

V3:
	$(CC) $(CFLAGS) -o $(BIN_DIR)/v3 $(SRC_DIR)/V3.cpp 

.PHONY: clean

all: V1 V2 V3

clean:
	rm -rf $(BIN_DIR)