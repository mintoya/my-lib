#include "arenaAllocator.h"
#include "hmap.h"
#include "macroList.h"
#include "print.h"
#include "umap.h"
#include "wheels.h"
#include <stddef.h>
#include <stdint.h>

int main(void) {
  Arena_scoped *local = arena_new(1024);
  List_scoped *hlist = List_new(local, sizeof(intmax_t));
  MList_DF(list, intmax_t, hlist);

  MList_push(list, 5);
  MList_push(list, 8);
  MList_push(list, 7);
  MList_push(list, 9);
  MList_push(list, 5);
  MList_push(list, 8);
  MList_push(list, 7);
  MList_push(list, 9);
  MList_foreach(list, index, element, { println("${}", (int)element); });

  HMap_scoped *hm = HMap_new(local, 10);
  HMap_set(hm, um_from("hello"), um_from("world"));
  HMap_set(hm, um_from("world"), um_from("hello"));
  println("${}", HMap_get(hm, um_from("hello")));
  UMap_scoped *um = UMap_new(local);
  UMap_set(um, um_from("hello"), um_from("world"));
  UMap_set(um, um_from("world"), um_from("hello"));
  println("${}", UMap_get(um, um_from("hello")));
  return 0;
}
