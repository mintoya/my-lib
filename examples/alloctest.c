#define PRINTER_LIST_TYPENAMES
#include "../arenaAllocator.h"
#include "../fptr.h"
#include "../hhmap.h"
#include "../hmap.h"
#include "../hmap_arena.h"
#include "../macroList.h"
#include "../print.h"
#include "../wheels.h"
#include <stddef.h>
#include <stdint.h>

int main(void) {
  My_allocator *local = hmap_alloc_new(
      arena_owned_new(),
      (struct hmap_alloc_opts){
          .summary = true,
          .fnshowOpts = last,
      }
  );
  do {
    char *l;
    List_scoped *hl = List_new(local, 5000);
    // assertMessage(hl != NULL, "nulltest");

    MList(int) list;
    MList_DFInit(list, local, hl); // overwrites hl

    MList_push(list, 5);
    MList_push(list, 8);
    MList_push(list, 7);
    MList_push(list, 9);
    MList_push(list, 5);
    MList_push(list, 8);
    MList_push(list, 7);
    MList_push(list, 9);
    MList_foreach(list, index, element, { println("${}", (int)element); });
    println("list footprint : ${}", List_headArea(MList_heapList(list)));

  } while (0);
  hmap_alloc_cleanup(local);
  return 0;
}
