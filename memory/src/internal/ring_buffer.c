#include "containers.h"

void index_ring_buffer_init(index_ring_buffer_t* rbuf, size_t capacity) {
  rbuf->capacity = capacity;
  rbuf->tail = 0;
  rbuf->head = 0;

  for (size_t i = 0; i < capacity; i++) {
    rbuf->indices[i] = i;
  }
}
