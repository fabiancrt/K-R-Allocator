# K-R-Allocator
A memory allocator built with the help of K&amp;R's C Programming book and CSAPP
book by Randal E. Bryant and David R. O’Hallaron.

This is a first-fit explicit allocator that uses a self built sbrk() function,
using Windows specific Virtual Memory Allocation and paging.

### Windows:
 For windows compilation use `mingw32-make all` in order to make use of the
 Makefile.

### Linux:
 For Linux compilation just simply use `make all` in order to make use of the
 Makefile. ( *NOTE*: you can change the compiler used in the compilation
         process inside the Makefile )
