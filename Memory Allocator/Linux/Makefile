#I use Clang to compile on Linux

CC = clang
FLAGS = -c -Wall -Wextra

all: malloc.o Allocator

malloc.o: malloc.c
	$(CC) $(FLAGS) malloc.c

Allocator: malloc.o
	$(CC) malloc.o -o Allocator

.PHONY: clean all

clean:
	rm -f *.o
	rm Allocator
