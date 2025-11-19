#include "../arenaAllocator.h"
#include "../fptr.h"
#include "../print.h"
#include "../stringList.h"
#include "../umap.h"
#include "../wheels.h"
#include <stddef.h>
#include <stdint.h>

int main(void) {
  Arena_scoped *local = arena_new(1000);
  UMap_scoped *um = UMap_new(local);
  UMap_set(um, fp_from("a"), fp_from("b"));
  UMap_setChild(um, fp_from("map"), um);
  println("${UMap}", *um);

  return 0;
}
