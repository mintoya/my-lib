#include "../omap.h"
#include "../arenaAllocator.h"
#include "../kmlM.h"
#include "../print.h"

#include "../printers/omap_printers.c"
#include "../wheels.h"
#include <stdint.h>

int main(void) {
  // Arena_scoped *local = arena_new(2500);
  My_allocator *local = arena_new(2500);

  OMap *omap = OMap_new(local);
  OMap_set(omap, fp_from("a"), fp_from("b"));
  OMap_set(omap, fp_from("b"), fp_from("c"));
  OMap_set(omap, fp_from("c"), fp_from("a"));
  stringList *sl = stringList_new(local);
  StringList_appendObj(sl, OMOJA(omap, OMAP));
  StringList_appendObj(sl, OMOJA(sl, SLIST));

  OMap_setObj(omap, fp_from("list"), OMOJA(sl, SLIST));
  OMap_setObj(omap, fp_from("recurse"), OMOJA(omap, OMAP));

  println("${OMap}", *omap);
  List_scoped *chars = mList(char);
  print_as(chars, "${OMap}", *omap);
  fptr printOutput = ((fptr){
      .width = chars->length,
      .ptr = chars->head,
  });
  print_wf(kmlFormatPrinter, "print output: ${}", printOutput);
  OMap_scoped *parsed = parse(NULL, NULL, printOutput);
  print_wf(kmlFormatPrinter, "parsed output: ${OMap}", *parsed);

  println("arena size: ${}", arena_footprint(local));
  arena_cleanup_handler(&local);
  return EXIT_SUCCESS;
}
