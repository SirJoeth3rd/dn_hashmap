#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "dn_vector.h"

/* A generic hashmap in C, implemented by considering every key, item as a group of bytes.
   Key assumed to be uint64_t, up to the user to convert their actual keys to u64
 */

// TODO: keep the actual items in a dn_vector, only keep {int distance, int key, void* in_vector_pos} struct in the hashmap.
#define MAX_PRIME_INDEX 20

static const unsigned int PRIMES[MAX_PRIME_INDEX + 1] = {
  1543,
  3079,
  6151,
  12289,
  24593,
  49157,
  98317,
  196613,
  393241,
  786433,
  1572869,
  3145739,
  6291469,
  12582917,
  25165843,
  50331653,
  100663319,
  201326611,
  402653189,
  805306457,
  1610612741
};

typedef struct {
  uint8_t distance;
  uint64_t key;
  void* data; // zero is a good null value now
} HashCell;

typedef struct {
  Vector item_vector;
  HashCell* cells;
  unsigned int prime_index;
  void* (*allocator)(size_t bytes);
  void (*collector)(void* buffer);
} HashMap;

HashMap hm_init(size_t item_size, allocator, collector);
HashMap hm_resize(HashMap map);
HashMap hm_put(HashMap map, uint64_t key, void* item);
HashMap hm_del(HashMap map, uint64_t key);
void* hm_get(HashMap map, uint64_t key);

static unsigned int closest_prime_index(unsigned int length) {
  for (unsigned int i = 0; i < 21; i++) {
    if (PRIMES[i] >= length) {
      return i;
    }
  }
  // TODO: error? maybe just raise a catastrophic crash
  return 0;
}

HashMap hm_init_vec(Vector v, allocator alloc_f, collector collect_f) {
  // if you need more granularity like different allocators for vector vs hashmap
  HashMap map;

  if (v.length > PRIMES[MAX_PRIME_INDEX]) {
    // TODO: how do we error this?
    //  I don't think we have too yet
  }
  
  for (unsigned int i = 0; i < 21; i++) {
    if (PRIMES[i] >= v.length) {
      map.prime_index = i;
      break;
    }
  }

  if (v.length != PRIMES[map.prime_index]) {
    v = vec_resize(v, PRIMES[map.prime_index]);
  }

  map.item_vector = v;
  map.allocator = alloc_f;
  map.collector = collect_f;
  map.cells = alloc_f(v.length * v.item_size);
  
  return map;
}

HashMap hm_init(size_t item_size, allocator alloc_f, collector collect_f) {
  HashMap map;
  map.prime_index = 0;
  map.item_vector = vec_init(PRIMES[map.prime_index], item_size, alloc_f, collect_f);
  map.allocator = alloc_f;
  map.collector = collect_f;
  return map;
}

void* hm_get(HashMap map, uint64_t key) {
  int distance = 0;
  int index = key % PRIMES[map.item_vector.length];

  while (!map.cells[index].data) {
    if (map.cells[index].key == key) {
      return map.cells[index].data;
    }
    if (map.cells[index].distance < distance) {
      return NULL;
    }
    distance++;
    index++;
  }
  return NULL;
}

HashMap hm_del(HashMap map, uint64_t key) {
  int distance = 0;
  int index = key % PRIMES[map.prime_index];
  
  while (!map.cells[index].data) {
    if (map.cells[index].key == key) {
      // TODO, also remove the item from the vector. Potentially as easy as just memcpying i+1 over i until end of vector
      // actually no that will invalidate every other cell, damn
      // wait wait it's still possible, loop over every cell and only decrement the data
      //  pointer if it's more then the delted position in the vector.
      //but deleting is actually REALLY expensive now. Should be fine though just don't
      // unless you really need to. Perhaps revisit the hashmap without vector dependency idea.
      /* memcpy(map.cells[index].data, map.cells[index].data + 1, map.cells[index].data); */
      memset(&map.cells[index], 0, sizeof(*map.cells));
      for (int i = index; map.cells[i+1].data; i++) {
	if (map.cells[i + 1].distance == 0) {
	  break;
	} else {
	  memcpy(&map.cells[i],&map.cells[i+1],sizeof(*map.cells));
	  map.cells[i].distance--;
	}
      }
      break;
    }

    if (map.cells[index].distance < distance) {
      break;
    }
    
    distance++;
    index++;
  }

  return map;
}

HashMap hm_put(HashMap map, uint64_t key, void* item) {
  if (map.item_vector.count > (unsigned int)(map.item_vector.length * 0.66)) {
    map = hm_resize(map);
  }

  unsigned int index = key % map.item_vector.length;
  HashCell tmpcell, currcell;

  currcell = map.cells[key % map.item_vector.length];

  while (map.cells[index].data || map.cells[index].key != key) {
    if (map.cells[index].distance > currcell.distance) {
      tmpcell = map.cells[index];
      map.cells[index] = currcell;
      currcell = tmpcell;
    }
    currcell.distance++;
    index++;
  }
  
  void* item_ptr = vec_add(&map.item_vector, item);
  map.item_vector.count++;

  currcell.data = item_ptr;
  map.cells[index] = currcell;
  
  return map;
}

HashMap hm_resize(HashMap map) {
  HashMap newmap;

  newmap.allocator = map.allocator;
  newmap.collector = map.collector;
  newmap.prime_index = map.prime_index + 1;
  newmap.item_vector = vec_resize(map.item_vector, PRIMES[newmap.prime_index]);
  newmap.cells = map.allocator(PRIMES[newmap.prime_index]);

  for (unsigned int i = 0; i < map.item_vector.length; i++) {
    if (map.cells[i].data) {
      newmap = hm_put(newmap, map.cells[i].key, map.cells[i].data);
    }
  }

  map.collector(map.cells);
  
  return newmap;
}

