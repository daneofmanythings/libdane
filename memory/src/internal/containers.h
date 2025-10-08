#ifndef MEMORY_INTERNAL_RING_BUFFER_H
#define MEMORY_INTERNAL_RING_BUFFER_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
  size_t capacity, head, tail;
  uint32_t indices[];
} index_ring_buffer_t;

void index_ring_buffer_init(index_ring_buffer_t* rbuf, size_t capacity);

#endif // MEMORY_INTERNAL_RING_BUFFER_H
