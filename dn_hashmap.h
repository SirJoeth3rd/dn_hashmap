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

typedef struct HashHead {
  void* data;
  uint64_t key;
  uint8_t distance;
} HashHead;

typedef struct HashMap {
  HashHead* keys;
  Vector vector;
  unsigned int prime_index;
  unsigned int capacity;
} HashMap;

HashMap hm_init(size_t);
HashMap hm_resize(HashMap map);
HashMap hm_put(HashMap map, uint64_t key, void* item);
HashMap hm_del(HashMap map, uint64_t key);
void* hm_get(HashMap map, uint64_t key);

HashMap hm_init(size_t item_size) {
  HashMap map;
  map.vector = vec_init(item_size);
  map.prime_index = 0;
  map.capacity = PRIMES[map.prime_index];
  map.keys = malloc(sizeof(HashHead)*map.capacity);
  return map;
}

void* hm_get(HashMap map, uint64_t key) {
  int distance = 0;
  int index = key % map.capacity;

  while (!map.keys[index].data) {
    if (map.keys[index].key == key) {
      return map.keys[index].data;
    }
    if (map.keys[index].distance < distance) {
      return NULL;
    }
    distance++;
    index++;
  }
  return NULL;
}

HashMap hm_del(HashMap map, uint64_t key) {
  int distance = 0;
  int index = key % map.capacity;

  while (map.keys[index].data) {
    if (map.keys[index].key == key) {
      map.keys[index] = (HashHead){0};
      for (int i = index; map.keys[i+1].data; i++) {
	if (map.keys[i+1].distance == 0) {
	  break;
	} else {
	  map.keys[i] = map.keys[i+1];
	  map.keys[i].distance--;
	}
      }
    }
    
    if (map.keys[index].distance < distance) {
      break;
    }
    
    distance++;
    index++;
  }
  //TODO: also delete the value out of the underlying vector and update every data pointer to it's new position
  return map;
}

HashMap hm_put(HashMap map, uint64_t key, void* item) {
  HashHead tmp, curr;
  
  if (map.vector.balance > map.capacity * 0.66) {
    //TODO: what's a better fraction than .66 here?
    hm_resize(map);
  }

  unsigned int index = key % map.capacity;
  curr = map.keys[index];

  while (map.keys[index].data || map.keys[index].key != key) {
    if (map.keys[index].distance > curr.distance) {
      tmp = map.keys[index];
      map.keys[index] = curr;
      curr = tmp;
    }
    curr.distance++;
    index++;
  }
  
  void* item_ptr = vec_add(&map.vector, item);

  curr.data = item_ptr;
  map.keys[index] = curr;
  
  return map;
}

HashMap hm_resize(HashMap map) {
  HashHead* oldkeys;
  unsigned int oldlength;
  oldkeys = map.keys;
  oldlength = map.capacity;
  
  map.prime_index = map.prime_index + 1;
  map.capacity = PRIMES[map.prime_index];
  map.keys = malloc(sizeof(HashHead)*map.capacity);

  for (unsigned int i = 0; i < oldlength; i++) {
    if (oldkeys[i].data) {
      map = hm_put(map, oldkeys[i].key, oldkeys[i].data);
    }
  }

  free(oldkeys);
  return map;
}

