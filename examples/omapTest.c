#include "../omap.h"
#include "../arenaAllocator.h"
#include "../kmlM.h"
#include "../print.h"
#include "../wheels.h"
#include <stdint.h>

int main(void) {
  Arena_scoped *local = arena_new(2500);

  // OMap *omap = OMap_new(local);
  // OMap_set(omap, fp_from("a"), fp_from("b"));
  // OMap_set(omap, fp_from("b"), fp_from("c"));
  // OMap_set(omap, fp_from("c"), fp_from("a"));
  // stringList *sl = stringList_new(local);
  // StringList_appendObj(sl, (OMap_ObjArg){.ptr.fptr = REF(fptr, fp_from("hello")), RAW});
  // StringList_appendObj(sl, (OMap_ObjArg){.ptr.fptr = REF(fptr, fp_from("world")), RAW});
  // StringList_appendObj(sl, (OMap_ObjArg){.ptr.optr = omap, OMAP});
  // StringList_appendObj(sl, (OMap_ObjArg){.ptr.sptr = sl, SLIST});
  //
  // // StringList_setObj(sl, 0, REF(fptr,fp_from()
  //
  // OMap_setObj(omap, fp_from("list"), (OMap_ObjArg){.ptr.sptr = sl, SLIST});
  // OMap_setObj(omap, fp_from("recurse"), (OMap_ObjArg){.ptr.optr = omap, OMAP});
  //
  // // OMapView oView = OMap_toView(local, omap);
  // println("${OMap}", *omap);
  // print_wf(kmlFormatPrinter, "${OMap}", *omap);
  // // OMap_free(omap);
  // // println("${OMapView}", oView);
  // println("arena size: ${}", arena_footprint(local));
  OMap_scoped *testparse = parse(NULL, NULL, fp_from("{hello:world;list:[test,item2,{in:here"));
  println("${OMap}", *testparse);
  return EXIT_SUCCESS;
}
