CC=gcc
CFLAGS=-Wall -O3 -fno-strict-aliasing

all: connection_handler.o btree.o stack.o
	$(CC) $(CFLAGS) stack.o btree.o connection_handler.o main.c -o unfile_server

clean:
	rm -f *.o unfile_server treetest

cleanhashes:
	rm -f hashes.dat

cleangrind:
	rm -f callgrind.out.*

testgrind:
	valgrind --tool=callgrind --dump-instr=yes --simulate-cache=yes --collect-jumps=yes ./unfile_server