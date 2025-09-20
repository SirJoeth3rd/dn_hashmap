#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>


/* A generic hashmap in C, implemented by considering every key, item as a group of bytes.
   Key assumed to be uint64_t, up to the user to convert their actual keys to u64
 */

#define MAX_ITEM_SIZE 64

static const int PRIMES[21] = {
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
} HashMeta;

typedef struct {
  void* buffer;
  size_t cell_size;
  unsigned int count;
  unsigned int prime_index;
  void* (*allocator)(size_t bytes);
  void (*collector)(void* buffer);
  uint8_t default_value[MAX_ITEM_SIZE];
} HashMap;

HashMap hashmap
(
 size_t item_size,
 void* (*allocator)(size_t bytes),
 void (*collector)(void* buffer)
 );

HashMap hm_resize(HashMap map);
HashMap hm_put(HashMap map, uint64_t key, void* item);
HashMap hm_del(HashMap map, uint64_t key);
void* hm_get(HashMap map, uint64_t key);

void hm_bytes_copy(void* dest, uint8_t* src, size_t len) {
    uint8_t* cdest = (uint8_t*)dest;

    for (size_t i = 0; i < len; i++) {
      cdest[i] = src[i];
    }
}

int hm_bytes_comp(uint8_t* a, uint8_t* b, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (a[i] != b[i]) {
      return 0;
    }
  }
  return 1;
}

HashMeta* hm_location_meta(HashMap map, int index) {
  return (HashMeta*)(map.buffer + index*map.cell_size);
}

void* hm_location_data(HashMap map, int index) {
  return (map.buffer + index*map.cell_size + sizeof(HashMeta));
}

int hm_is_default_val(HashMap map, int index) {
  uint8_t* data = hm_location_data(map, index);
  return hm_bytes_comp(data, map.default_value, map.cell_size - sizeof(HashMeta));
}

void* hm_get(HashMap map, uint64_t key) {
  int distance = 0;
  int index = key % PRIMES[map.count];

  while (!hm_is_default_val(map, index)) {
    if (hm_location_meta(map, index)->key == key) {
      return hm_location_data(map, index);
    }
    if (hm_location_meta(map, index)->distance < distance) {
      return NULL;
    }
    distance++;
    index += map.cell_size;
  }

  return NULL;
}

HashMap hm_del(HashMap map, uint64_t key) {
  int distance = 0;
  int index = key % PRIMES[map.prime_index];

  while (!hm_is_default_val(map, index)) {
    if (hm_location_meta(map, index)->key == key) {
      map.count--;
      hm_bytes_copy(hm_location_data(map, index), map.default_value, map.cell_size - sizeof(HashMeta));
      break;
    }
    if (hm_location_meta(map, index).distance < distance) {
      break;
    }
    distance++;
    index += map.cell_size;
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

