#include "../omap.h"
#include "../arenaAllocator.h"
#include "../kmlM.h"
#include "../print.h"

#include "../printers/omap_printers.c"
#define B_STACKTRACE_IMPL

#include "../b_stacktrace/b_stacktrace.h"
#include "../hmap_arena.c"
#include "../wheels.h"
#include <stdint.h>

int main(void) {
  My_allocator *local = hmap_alloc_new(arena_owned_new());
  do {
    OMap_scoped *omap = OMap_new(local);
    OMap_set(omap, fp_from("a"), fp_from("b"));
    OMap_set(omap, fp_from("b"), fp_from("c"));
    OMap_set(omap, fp_from("c"), fp_from("a"));
    stringList_scoped *sl = stringList_new(local);
    StringList_appendObj(sl, OMOJA(omap, OMAP));
    StringList_appendObj(sl, OMOJA(sl, SLIST));

    OMap_setObj(omap, fp_from("list"), OMOJA(sl, SLIST));
    OMap_setObj(omap, fp_from("recurse"), OMOJA(omap, OMAP));

    println("${OMap}", *omap);
    List_scoped *chars = mList(local, char);
    print_as(chars, "${OMap}", *omap);
    fptr printOutput = ((fptr){
        .width = chars->length,
        .ptr = chars->head,
    });
    print_wf(kmlFormatPrinter, "print output: ${}", printOutput);
    OMap_scoped *parsed = parse(NULL, NULL, printOutput);
    print_wf(kmlFormatPrinter, "parsed output: ${OMap}", *parsed);
  } while (0);
  hmap_alloc_cleanup(local);
  return EXIT_SUCCESS;
}
