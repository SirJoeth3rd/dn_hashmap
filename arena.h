/*
  Arena allocator implementation

  This was originally tsoding's that I mangled into my own

  Some specifics about this implementation.
  -- It's untyped, meaning it's up to the user to provide the size to the arena
  -- Also defined in here is the Allocator struct, this is not specific to the Arena.
 */

#ifndef DN_ARENA
#define DN_ARENA

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

static const size_t PAGE_SIZE = 4096;
// TODO: remove this assumption PAGE_SIZE = 4096

struct Region {
  void* buffer;
  size_t balance; // bytes already used
  size_t capacity; // total bytes
  struct Region* next;
};

typedef struct Region Region;

typedef struct {
  Region* start;
  Region* end;
} Arena;

void arena_append_region(Arena*);
void* arena_alloc(Arena*, size_t);
void arena_free(Arena*);
Arena arena_init();

void* arena_alloc(Arena* arena, size_t bytes) {
  void* ptr;
  if (bytes > PAGE_SIZE) {
    printf("Tried to allocate more bytes then page size in allocator\n");
    abort();
  }
  
  if (arena->end->capacity - arena->end->balance < bytes) {
    arena_append_region(arena);
  }

  ptr = arena->end->buffer + arena->end->balance;
  arena->end->balance += bytes;
  return ptr;
}

void arena_append_region(Arena* arena) {
  Region* new_region;
  new_region = malloc(sizeof(*new_region));
  arena->end->next = new_region;
  arena->end = new_region;
  arena->end->buffer = malloc(PAGE_SIZE*sizeof(char));
  arena->end->balance = 0;
  arena->end->capacity = PAGE_SIZE;
}

void arena_free(Arena* arena) {
  Region* current_region;
  current_region = arena->start;
  while (current_region) {
    free(current_region->buffer);
    free(current_region);
    current_region = current_region->next;
  }
}

Arena arena_init() {
  Arena arena;
  Region* first;
  first = malloc(sizeof(first));
  arena.start = first;
  arena.start->buffer = malloc(PAGE_SIZE*sizeof(char));
  arena.start->capacity = PAGE_SIZE;
  arena.start->balance = 0;
  arena.end = arena.start;
  return arena;
}

#endif // DN_ARENA
