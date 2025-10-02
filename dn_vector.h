/*
  Dynamic array implementation for C.
  This has been solved some times, but if we never reinvented the wheel we would never have had the merry-go-round.
 */

#include <stddef.h>
#include <string.h>

const unsigned int VEC_GROWTH_FACTOR = 2;

typedef void* (*allocator)(size_t bytes);
typedef void (*collector)(void* buffer);

typedef struct {
  size_t item_size;
  unsigned int length;
  unsigned int count;
  void *buffer;
  void* (*allocator)(size_t bytes);
  void (*collector)(void* buffer);
} Vector;

Vector vec_init(int initial_length, size_t item_size, allocator, collector);
void* vec_add(Vector*, void*);
void vec_del(Vector, int);
Vector vec_resize(Vector, unsigned int);
void* vec_get(Vector, int);

Vector vec_init(int initial_length, size_t item_size, void* (*allocator)(size_t bytes), void (*collector)(void* buffer)) {
  Vector v;
  if (initial_length == 0) {
    v.length = 1;
  } else {
    v.length = initial_length;
  }

  v.allocator = allocator;
  v.collector = collector;

  v.buffer = allocator(v.length * item_size);
  v.count = 0;

  return v;
}

Vector vec_resize(Vector v, unsigned int new_length) {
  Vector new_vec;
  new_vec.item_size = v.item_size;
  new_vec.allocator = v.allocator;
  new_vec.collector = v.collector;
  new_vec.length = new_length;
  new_vec.buffer = new_vec.allocator(new_vec.item_size * new_vec.length);
  memcpy(new_vec.buffer, v.buffer, new_vec.item_size * v.count);
  v.collector(v.buffer);
  new_vec.count = v.count;
  return new_vec;
}

void* vec_add(Vector* vector, void* item) {
  void* buffer_loc;
  if (vector->count > vector->length - 1) {
    *vector = vec_resize(*vector, (int)(vector->length * VEC_GROWTH_FACTOR));
  }
  buffer_loc = vector->buffer + vector->count*vector->item_size;
  memcpy(buffer_loc, item, vector->item_size);
  vector->count++;
  return buffer_loc;
}

void vec_del(Vector vector, int index) {
  for (unsigned int j = index; j < vector.count - 1; j++) {
    memcpy(vector.buffer + j*vector.item_size, vector.buffer + (j+1)*vector.item_size, vector.item_size);
  }
}

void* vec_get(Vector vector, int index) {
  return vector.buffer + index*vector.item_size;
}
