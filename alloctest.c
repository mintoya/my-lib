// #define PRINTER_LIST_TYPENAMES
#include "allocator.h"
#include "arenaAllocator.h"
#include "hmap.h"
#include "macroList.h"
#include "print.h"
#include "umap.h"
#include "wheels.h"
#include <stddef.h>
#include <stdint.h>

int main(void) {
  Arena_scoped *local = arena_new(1500);
  List *hl;
  MList(int) list;
  MList_DFInit(list, local, hl);

  MList_push(list, 5);
  MList_push(list, 8);
  MList_push(list, 7);
  MList_push(list, 9);
  MList_push(list, 5);
  MList_push(list, 8);
  MList_push(list, 7);
  MList_push(list, 9);
  MList_foreach(list, index, element, { println("${}", (int)element); });
  println("list footprint : ${}", List_headArea(hl));
  List_free(hl);

  HMap *hm = HMap_new(local, 10);
  UMap *um = UMap_new(local);

  HMap_set(hm, um_from("hello"), um_from("world"));
  HMap_set(hm, um_from("world"), um_from("hello"));
  HMap_set(hm, um_from("lsajdf"), um_from("world"));
  HMap_set(hm, um_from("askjdf"), um_from("hello"));

  println("${}", HMap_get(hm, um_from("hello")));
  println("${}", HMap_get(hm, um_from("world")));
  println("hmap footprint : ${}", HMap_footprint(hm));
  HMap_free(hm);

  UMap_set(um, um_from("hello"), um_from("world"));
  UMap_set(um, um_from("world"), um_from("hello"));

  UMap_set(um, um_from("heliafj"), um_from("world"));
  UMap_set(um, um_from("asvd k"), um_from("hello"));

  println("${}", UMap_get(um, um_from("world")));
  println("${}", UMap_get(um, um_from("hello")));

  println("umap footprint : ${}", UMap_footprint(um));
  println("allocated area : ${}", arena_footprint(local));

  um_fp bufferTest = (um_fp){
      .ptr = aAlloc(local, 50),
      .width = 20,
  };
  print_sn(bufferTest, "he${}lo world ${}", 'l', bufferTest.width);
  println("${}", bufferTest);
  print_sn(bufferTest, "lkjskdjfksdfjslfkjslkjflskjksflj");
  println("${}", bufferTest);

  return 0;
}
