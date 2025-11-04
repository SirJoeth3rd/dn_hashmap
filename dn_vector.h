/*
  Dynamic array implementation for C.
  This has been solved some times, but if we never reinvented the wheel we would never have had the merry-go-round.
 */

#include <stddef.h>
#include <string.h>
#include "arena.h"

const unsigned int VEC_GROWTH_FACTOR = 2;

typedef struct {
  size_t item_size;
  unsigned int balance;
  Arena arena;
} Vector;

Vector vec_init(size_t);
Vector vec_init_arena(size_t, Arena);
void* vec_add(Vector*, void*);
void vec_del(Vector, int);
Vector vec_resize(Vector, unsigned int);
void* vec_get(Vector, int);

Vector vec_init_arena(size_t item_size, Arena arena) {
  Vector v;
  v.item_size = item_size;
  v.arena = arena;
  return v;
}

Vector vec_init(size_t item_size) {
  Vector v;
  
  v.item_size = item_size;
  v.arena = arena_init();
  v.balance = 0;

  return v;
}

void* vec_add(Vector* vector, void* item) {
  void* location;
  location = arena_alloc(&vector->arena, vector->item_size);
  memcpy(location, item, vector->item_size);
  vector->balance++;
  return location;
}

// deleting and retrieval is a bit more difficult if you are a Array/LinkedList Abomination

void* vec_get(Vector vector, int index) {
  int num_items_in_page, page_num;
  Region* target_region;
  num_items_in_page = PAGE_SIZE / vector.item_size;
  page_num = num_items_in_page / index;

  target_region = vector.arena.start;
  while (page_num) {
    target_region = target_region->next;
    page_num--;
  }

  return target_region->buffer + (index - (page_num*PAGE_SIZE))*vector.item_size;
}

// TODO: vec_del
