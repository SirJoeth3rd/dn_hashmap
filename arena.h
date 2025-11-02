/*
  Arena allocator implementation

  This was originally tsoding's that I mangled into my own
 */

#include <stddef.h>
#include <stdlib.h>

struct Region {
  void* buffer;
  size_t capacity;
  size_t item_size;
  struct Region* next;
};

typedef struct Region Region;

typedef struct {
  Region* start;
  Region* end;
} Arena;

typedef struct {
  size_t item_size;
  Arena arena;
} TypedArena;

// this is arena where the underlying datastructure is a linked list

typedef struct {
  void* (*allocate)(void* context, size_t size);
  void (*deallocate)(void* context, void* buffer);
  void* context;
} Allocator;

void* arena_alloc(Arena* arena, size_t item_size, size_t count) {
  void* buffer = malloc(item_size*count);
  Region* new_region = malloc(sizeof(Region));
  
  new_region->buffer = buffer;
  new_region->item_size = item_size;
  new_region->capacity = count;
  new_region->next = NULL;
  
  if (!arena->start) {
    arena->start = new_region;
  } else if (!arena->end) {
    arena->end = new_region;
  } else {
    arena->end->next = new_region;
    arena->end = new_region;
  }

  return buffer;
}

void* typed_arena_alloc(TypedArena* arena, size_t count) {
  return arena_alloc(&arena->arena, arena->item_size, count);
}

void arena_free(Arena* arena) {
  Region* current_region;
  current_region = arena->start;
  while (current_region) {
    free(current_region->buffer);
    current_region = current_region->next;
  }
}

void arena_free_region(Arena* arena, void* region) {
  current_region
}

void* typed_arena_allocate(void* arena, size_t count) {
  return typed_arena_alloc((TypedArena*)arena, count);
}

void typed_arena_deallocate(void* arena, void* buffer) {
  return 
}

Allocator typed_arena_allocator(TypedArena* arena, size_t count) {
  return (Allocator){
    .context = arena,
    .allocate = typed_arena_alloc,
    .deallocate = 
  };
}

void* malloc_allocate(void* context, size_t size) {
  // just used to ignore the context pointer
  return malloc(size);
}

Allocator malloc_allocator() {
  return (Allocator){
    .allocate = malloc_allocate,
    .context = NULL
  };
}
