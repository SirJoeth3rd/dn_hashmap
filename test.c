#include "arena.h"
#include "dn_hashmap.h"
#include "dn_vector.h"

#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

int main() {
  Vector int_vector;
  int_vector = vec_init(sizeof(int));

  for (int i = 0; i < 10000; i++) {
    vec_add(&int_vector, &i);
    assert(*((int*)vec_get(int_vector, i))==i);
  }

  printf("dn_vector tests completed\n");

  HashMap int_map;
  int_map = hm_init(sizeof(int));

  for (int i = 0; i <10000; i++) {
    hm_put(&int_map, ((uint64_t)i)<<22, &i);
    assert(*((int*)hm_get(&int_map, ((uint64_t)i)<<22)) == i);
  }

  printf("dn_hashmap tests completed\n");
}
