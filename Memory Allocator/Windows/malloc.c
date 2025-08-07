/* My allocator is a first-fit explicit allocator */
/*Built with the help of the guides in these books : C The Programming Language and CS:APP */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <errno.h>
#include <time.h>
#include <inttypes.h>
#include "sbrk.h"

#define MIN_ALLOCATION 1024
#define PACK(size, alloc) ((size) | (alloc))
#define GET_SIZE(p) ((p) & ~0x7)
#define GET_ALLOC(p) ((p) & 0x1)

#define GET_MARK(p) (((p) & 0x2) >> 1)
#define SET_MARK(p) ((p) | 0x2)
#define CLEAR_MARK(p) ((p) & ~0x2)
#define PACK_MARK(size, alloc, mark) ((size) | (alloc) | ((mark) << 1))

typedef long Align;
typedef void *ptr;

union header {
	struct {
		union header *ptr;
		unsigned int size_and_alloc;
	} s;
	Align x;
};
typedef union header Header;

static Header baseOfList;
static Header *freeLstStartP = NULL;

static void set_footer(Header *blockH, unsigned int size_alloc) {
	Header *footer = blockH + GET_SIZE(size_alloc) - 1;
	footer->s.size_and_alloc = size_alloc;
}

void my_free(void *block) {
	Header *cursor, *blockH;
	blockH = (Header*) block - 1;
	/*First we set free*/
	unsigned int size = GET_SIZE(blockH->s.size_and_alloc);
	blockH->s.size_and_alloc = PACK(size, 0); /*sets it free */
	set_footer(blockH, blockH->s.size_and_alloc);

	/*Moving the cursor to a correct insertion point */
	for (cursor = freeLstStartP; !(blockH > cursor && blockH < cursor->s.ptr);
			cursor = cursor->s.ptr)
		if (cursor >= cursor->s.ptr
				&& (blockH > cursor || blockH < cursor->s.ptr))
			break;
	Header *next_block = blockH + size;
	if (next_block == cursor->s.ptr && !GET_ALLOC(next_block->s.size_and_alloc)) {
		blockH->s.size_and_alloc = PACK(
				size + GET_SIZE(cursor->s.ptr->s.size_and_alloc), 0);
		blockH->s.ptr = cursor->s.ptr->s.ptr;/*skip over the merged block*/
		set_footer(blockH, blockH->s.size_and_alloc);
	} else {
		blockH->s.ptr = cursor->s.ptr; /* normal insertion no coalesce */
	}
	if (cursor + GET_SIZE(cursor->s.size_and_alloc)
			== blockH&& !GET_ALLOC(cursor->s.size_and_alloc)) {
		cursor->s.size_and_alloc =
				PACK(
						GET_SIZE(cursor->s.size_and_alloc) + GET_SIZE(blockH->s.size_and_alloc),
						0);
		cursor->s.ptr = blockH->s.ptr; /* cursor points to what blockH points to */
		set_footer(cursor, cursor->s.size_and_alloc);
	} else {
		cursor->s.ptr = blockH;
	}
}

void* moreMem(unsigned units) {
	char *charP;
	Header *up;
	if (units < MIN_ALLOCATION)
		units = MIN_ALLOCATION;
	charP = sbrk(units * sizeof(Header));
	if (charP == (char*) -1) {
		return NULL;
	}
	up = (Header*) charP;
	up->s.size_and_alloc = PACK(units, 0); /*changed */
	my_free((void*) (up + 1));
	return freeLstStartP;
}

void* my_malloc(unsigned bytes) {
	Header *cursor, *prevp;
	unsigned units;
	units = (bytes + sizeof(Header) - 1) / sizeof(Header) + 1;

	if (units % 2)
		units++; /*new*/

	if (freeLstStartP == NULL) {
		baseOfList.s.ptr = freeLstStartP = prevp = &baseOfList;
		baseOfList.s.size_and_alloc = PACK(0, 1); /*changed*/
	}
    else{
        prevp = freeLstStartP;
    }
	for (cursor = prevp->s.ptr;; prevp = cursor, cursor = cursor->s.ptr) {
		unsigned int cursor_size = GET_SIZE(cursor->s.size_and_alloc);
		if (cursor_size >= units) {
			if (cursor_size == units) {
				prevp->s.ptr = cursor->s.ptr;
				cursor->s.size_and_alloc = PACK(units, 1); /*changed: mark as allocated */
			} else if (cursor_size - units >= 2) { /* changed: only split if the remainder is useful */
				cursor->s.size_and_alloc = PACK(cursor_size - units, 0);
				cursor += cursor_size - units; /*leave the tail behind*/
				cursor->s.size_and_alloc = PACK(units, 1);
				set_footer(cursor - (cursor_size - units),
						PACK(cursor_size - units, 0));
			} else {
				/* use the whole size */
				prevp->s.ptr = cursor->s.ptr;
				cursor->s.size_and_alloc = PACK(cursor_size, 1);
			}
			freeLstStartP = prevp; /*moved to the gained block*/
			return (void*) (cursor + 1);
		}
		if (cursor == freeLstStartP) {
			if ((cursor = moreMem(units)) == NULL) {
				errno = ENOMEM;
				return NULL;
			}
		}
	}
	printf("Unexpected behavior inside malloc.\n");
	return NULL; /*never reached*/
}

int main() {
	int (*ptr)[6] = my_malloc(6 * sizeof(*ptr));
	int i, j;

	if (!ptr) {
		printf("Failed Malloc!");
		return 1;
	}

	for (i = 0; i < 6; i++)
		for (j = 0; j < 6; j++)
			ptr[i][j] = i + j;

	printf("Good spatial locality:\n"
			"i:j -> linear index -> address\n\n");
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 6; j++)
			printf("%d:%d -> %2d -> %p ", i, j, (i * 6 + j),
					(void*) &ptr[i][j]);
		printf("\n");
	}
	printf("\n");

	printf("Poor spatial locality:\n\n");
	/* if array > cache => each ptr[i][j] will miss */
	for (j = 0; j < 6; j++) {
		for (i = 0; i < 6; i++)
			printf("%d:%d -> %2d -> %p ", i, j, (i * 6 + j),
					(void*) &ptr[i][j]);
		printf("\n");
	}
	printf("\n");

	my_free(ptr);
	return 0;
}

