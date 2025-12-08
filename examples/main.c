#define PRINTER_LIST_TYPENAMES
#include "../arenaAllocator.h"
#include "../hhmap.h"
#include "../hmap_arena.h"
#include "../print.h"
#include "../wheels.h"

#include <stdio.h>
#include <string.h>

int main(void) {
  My_allocator *local = hmap_alloc_new(
      arena_owned_new(),
      (struct hmap_alloc_opts){
          .summary = true,
          .footprint = true,
          .fnshowOpts = first,
      }
  );
  // Arena_scoped *local = arena_new();

  println("=== HHMap Test Suite ===\n");

  // Test 4: Force collisions with small metaSize
  println("Test 4: Collision handling");
  HHMap *small_map = HHMap_new(sizeof(int), sizeof(int), local, 4);

  println("  Inserting keys and showing hash distribution:");
  for (int i = 0; i < 10; i++) {
    int k = i * 100;
    int v = i;
    unsigned int hash = HHMap_hash((fptr){sizeof(int), (u8 *)&k}) % 4;
    u32 idx = HHMap_set(small_map, &k, &v);
    println("key=${}, hash_slot=${}, stored_at_index=${}", k, hash, idx);
    assertMessage(*(int *)HHMap_getKey(small_map, idx) == k, "hmap returned wrong index?");
    assertMessage(*(int *)HHMap_getVal(small_map, idx) == v, "hmap returned wrong index?");
  }
  println();
  assertMessage(HHMap_count(small_map) == 10);
  for (int i = 0; i < 10; i++) {
    int k = *(int *)HHMap_getKey(small_map, i);
    int *v = (int *)HHMap_get(small_map, &k);
    assertMessage(v && *v == k / 100, "wrong key retrieved");
    println("${} -> ${}", k, *v);
  }

  println("=== All tests complete ===");
  hmap_alloc_cleanup(local);
  return 0;
}
