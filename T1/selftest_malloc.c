#include <stdlib.h>
#include <stdio.h>

void *malloc_t1(size_t size) {
	return malloc(size);
}

void free_t1(void *ptr) {
	free(ptr);
}
