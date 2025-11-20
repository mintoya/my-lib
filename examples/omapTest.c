#include "../omap.h"
#include "../arenaAllocator.h"
#include "../kmlM.h"
#include "../print.h"
#include "../umap.h"
#include "../wheels.h"
#include <stdint.h>

int main(void) {
  Arena_scoped *local = arena_new(1000);

  OMap *omap = OMap_new(local);
  OMap_set(omap, fp_from("a"), fp_from("b"));
  OMap_set(omap, fp_from("b"), fp_from("c"));
  OMap_set(omap, fp_from("c"), fp_from("a"));
  stringList *sl = stringList_new(local);
  StringList_appendObj(sl, REF(fptr, fp_from("hello")), RAW);
  StringList_appendObj(sl, REF(fptr, fp_from("world")), RAW);
  StringList_appendObj(sl, sl, SLIST);
  StringList_appendObj(sl, omap, OMAP);

  OMap_setObj(omap, fp_from("list"), sl, SLIST);
  OMap_setObj(omap, fp_from("recurse"), omap, OMAP);

  // OMapView oView = OMap_toView(local, omap);
  println("${OMap}", *omap);
  print_wf(kmlFormatPrinter, "${OMap}", *omap);
  // OMap_free(omap);
  // println("${OMapView}", oView);
  return EXIT_SUCCESS;
}
