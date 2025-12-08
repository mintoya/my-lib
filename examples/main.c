#include "../arenaAllocator.h"
#include "../hhmap.h"
#include "../hmap_arena.h"
#define PRINTER_LIST_TYPENAMES
#include "../print.h"
#include "../wheels.h"
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

  println("Test 4: Collision handling");

  HHMap *small_map = HHMap_new(sizeof(int), sizeof(int), local, 4);
  println("  Inserting keys and showing hash distribution:");
  for (int i = 0; i < 10; i++) {
    int v = i * i;
    HHMap_set(small_map, &i, &v);
  }
  assertMessage(HHMap_count(small_map) == 10);
  bool all = true;
  for (int i = 0; i < 10; i++) {
    int v;
    HHmap_getSet(small_map, &i, &v);
    valFullElse(
        HHmap_getSet(small_map, &i, &v),
        { all = false;v = -999; },
        false
    );
    println("[${}]:${}->${}", i, i, v);
  }
  assertMessage(all, "hhmap couldnt retrieve all values ");

  println("=== All tests complete ===");
  hmap_alloc_cleanup(local);
  return 0;
}
