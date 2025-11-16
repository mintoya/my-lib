#define PRINTER_LIST_TYPENAMES
#include "../allocator.h"
#include "../arenaAllocator.h"
#include "../fptr.h"
#include "../hmap.h"
#include "../macroList.h"
#include "../print.h"
#include "../umap.h"
#include "../wheels.h"
#include <stddef.h>
#include <stdint.h>

int main(void) {
  char *l;
  Arena_scoped *local = arena_new(200);
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
  println("list footprint : ${}", List_headArea(MList_heapList(list)));
  List_free(hl);
  return 0;
}
