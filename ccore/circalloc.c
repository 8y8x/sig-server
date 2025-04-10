#include <assert.h>
#include <stdlib.h>

#include "circalloc.h"

circalloc_t circalloc(size_t initial_capacity, size_t item_size) {
	void* items = malloc(item_size * initial_capacity);
	void** pointers = malloc(sizeof(size_t) * initial_capacity);

	for (size_t i = 0; i < initial_capacity; ++i) {
		pointers[i] = items + i * item_size;
	}

	circalloc_t ca = { pointers, items, item_size, initial_capacity - 1, 0, initial_capacity - 1 };
	return ca;
}

void circalloc_destroy(circalloc_t* ca) {
	free(ca->pointers); // ->items is stored after ->pointers in the same block
}

void* circalloc_alloc(circalloc_t* ca) {
	assert(ca->pointers_front != ca->pointers_back);

	void* item = ca->pointers[ca->pointers_front++];
	ca->pointers_front &= ca->capacity_mask;
	return item;
}

void circalloc_free(circalloc_t* ca, void* item) {
	++ca->pointers_back;
	ca->pointers_back &= ca->capacity_mask;

	assert(ca->pointers_front != ca->pointers_back);

	ca->pointers[ca->pointers_back] = item;
}
