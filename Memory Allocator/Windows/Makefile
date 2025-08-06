#MinGW 

CC = gcc
FLAGS = -c -Wall -Wextra -g

all: malloc.o sbrk.o Allocator

malloc.o: malloc.c
	$(CC) $(FLAGS) malloc.c

sbrk.o: sbrk.c
	$(CC) $(FLAGS) sbrk.c

clean:
	del Allocator.exe malloc.o sbrk.o

.PHONY: all clean
