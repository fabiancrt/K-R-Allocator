#include <errno.h>
#include <windows.h>
#include <stdint.h>
#ifndef HEAP_RESERVE_SIZE
/* 16 MiB */
#define HEAP_RESERVE_SIZE (16 * 1024 * 1024)
#endif
typedef uint8_t Heap;
static Heap *heap_base = NULL;
static Heap *heap_end = NULL;
static Heap *heap_limit = NULL;
void *sbrk(intptr_t bytesIncrement) {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	size_t pageSize = si.dwPageSize;
	if (heap_base == NULL) {
		heap_base = VirtualAlloc(NULL,
		HEAP_RESERVE_SIZE, /* 16 MiB exactly */
		MEM_RESERVE,
		PAGE_READWRITE);
		if (!heap_base) {
			return NULL;
		}
		heap_end = heap_base;
		heap_limit = heap_base + HEAP_RESERVE_SIZE;
	}
	if (bytesIncrement == 0) {
		return heap_end;
	}
	Heap *old_break = heap_end;
	Heap *new_break = old_break + bytesIncrement;

	if (new_break > heap_limit) {
		errno = ENOMEM;
		return (void*) -1;
	}

	if (bytesIncrement > 0) {
		uintptr_t needed = (uintptr_t) new_break - (uintptr_t) old_break;
		size_t commit_size = (size_t) (needed + (uintptr_t) pageSize - 1)
				& ~(pageSize - 1);
		if (commit_size)
			if (!(VirtualAlloc(old_break, /*last end address*/
			commit_size, /*size*/
			MEM_COMMIT, /*to reserve the memory*/
			PAGE_READWRITE /* RW permissions */
			))) {
				errno = ENOMEM;
				return (void*) -1;
			}
	} else if (bytesIncrement < 0) {
		uintptr_t whereOnPage = ((uintptr_t) new_break + pageSize - 1)
				& ~(pageSize - 1);
		if (whereOnPage < (uintptr_t) old_break) {
			size_t decommit_size = (size_t)((uintptr_t) old_break - whereOnPage)
					& ~((size_t) pageSize - 1);
			if (decommit_size)
				if (!(VirtualFree((LPVOID) whereOnPage, /*new address after decommit*/
				decommit_size, /*size*/
				MEM_DECOMMIT /*to reserve the memory*/
				))) {
					errno = ENOMEM;
					return (void*) -1;
				}
		}
	}
	heap_end = new_break;
	return old_break;
}
