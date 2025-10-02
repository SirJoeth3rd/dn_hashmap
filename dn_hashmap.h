#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "dn_vector.h"

/* A generic hashmap in C, implemented by considering every key, item as a group of bytes.
   Key assumed to be uint64_t, up to the user to convert their actual keys to u64
 */

// TODO: keep the actual items in a dn_vector, only keep {int distance, int key, void* in_vector_pos} struct in the hashmap.
#define MAX_PRIME_INDEX 21

static const unsigned int PRIMES[MAX_PRIME_INDEX] = {
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
  if (map.count > (unsigned int)(PRIMES[map.prime_index] * 0.66)) {
    map = hm_resize(map);
  }
  
  int index = key % PRIMES[map.prime_index]; // the hash
  HashMeta meta = *hm_location_meta(map, index);
  HashMeta tmp_meta;
  HashMeta* location_meta;
  
  uint8_t item_buffer[map.cell_size - sizeof(HashMeta)];
  hm_bytes_copy(item_buffer, item, map.cell_size - sizeof(HashMeta));
  uint8_t tmp_item_buffer[map.cell_size - sizeof(HashMeta)];

  while (!hm_is_default_val(map, index)) {
    location_meta = hm_location_meta(map, index);
    if (meta.distance > location_meta->distance) {
      void* data_location = hm_location_data(map, index);
      hm_bytes_copy(tmp_item_buffer, data_location, map.cell_size - sizeof(HashMeta));
      hm_bytes_copy(data_location, item_buffer, map.cell_size - sizeof(HashMeta));
      hm_bytes_copy(item_buffer, tmp_item_buffer, map.cell_size - sizeof(HashMeta));

      tmp_meta = *location_meta;
      *location_meta = meta;
      meta = tmp_meta;
    }
    meta.distance++;
    index++;
  }

  hm_bytes_copy(hm_location_data(map, index),item_buffer, map.cell_size - sizeof(HashMeta));
  *hm_location_meta(map, index) = meta;
  map.count++;
  return map;
}

HashMap hm_resize(HashMap map) {
  HashMap new_map;
  
  new_map.prime_index = map.prime_index + 1;
  new_map.cell_size = map.cell_size;
  new_map.allocator = map.allocator;
  new_map.collector = map.collector;
  hm_bytes_copy(new_map.default_value, map.default_value, MAX_ITEM_SIZE);
  
  new_map.buffer = map.allocator(map.cell_size * PRIMES[new_map.prime_index]);
  map.collector(map.buffer);
  
  new_map.count = 0;
  for (int i = 0; i < PRIMES[map.prime_index]; i++) {
    if (!hm_is_default_val(map, i)) {
      new_map = hm_put(new_map, hm_location_meta(map, i)->key,hm_location_data(map, i));
    }
  }
  
  return new_map;
}

