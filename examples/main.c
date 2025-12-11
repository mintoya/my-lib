#include "../arenaAllocator.h"
#include "../hhmap.h"
#include "../hmap_arena.h"
#include "../print.h"
#include "../wheels.h"

int main(void) {
  // My_allocator *local = hmap_alloc_new(arena_owned_new(), (struct hmap_alloc_opts){0});
  Arena_scoped *local = pageArena_new();

  HHMap_scoped *small_map = HHMap_new(sizeof(int), sizeof(int), local, 4);
  for (int i = 0; i < 10; i++) {
    int v = i * i;
    HHMap_set(small_map, &i, &v);
  }
  assertMessage(HHMap_count(small_map) == 10);
  bool all = true;
  for (int i = 0; i < 10; i++) {
    int v;
    HHMap_getSet(small_map, &i, &v);
    valFullElse(
        HHMap_getSet(small_map, &i, &v),
        { all = false;v = -999; },
        false
    );
    println("[{}]:{}->{}", i, i, v);
  }
  assertMessage(all, "hhmap couldnt retrieve all values ");

  LList_head *l = LList_new(local, sizeof(int));
  LList_push(l, REF(int, 1));
  LList_push(l, REF(int, 2));
  LList_push(l, REF(int, 3));

  LList_element *e = l->first;
  while (e) {
    println("{}", *(int *)e->data);
    e = e->next;
  }
  return 0;
}
