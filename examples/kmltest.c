#include <alloca.h>
#define PRINTER_LIST_TYPENAMES
#include "../allocator.h"
#include "../kmlM.h"
#include "../my-list.h"
#include "../print.h"
#include "../umap.h"
#include "../wheels.h"

int main() {
  // UMap_scoped *test = UMap_new(&defaultAllocator);
  // UMap_set(test, fp_from("a"), fp_from("b"));
  // UMap_set(test, fp_from("b"), fp_from("c"));
  // UMap_set(test, fp_from("c"), fp_from("a"));
  // UMap_setChild(test, fp_from("object"), test);
  // UMapList_scoped *tlist = UMapList_new(&defaultAllocator);
  // UMapList_append(tlist, fp_from("one"));
  // UMapList_append(tlist, fp_from("two"));
  // UMapList_append(tlist, fp_from("three"));
  // UMap_setList(test, fp_from("list"), tlist);
  //
  // List_scoped *bufferList = NULL;
  // print_as(bufferList, "${UMap}", *test);
  // UMap_scoped *output = parse(
  //     NULL, NULL,
  //     ((fptr){List_headArea(bufferList), bufferList->head})
  // );
  char text[] = {
#embed "../kml/elements.kml"
  };
  UMap_scoped *output = parse(NULL, NULL, fp_from(text));
  UMapView elementsEl = UMap_get(output, fp_from("elements"));
  print_wf(kmlFormatPrinter, "${UMap}", *output);

  ffptr pbuffer = (ffptr){
      .ffptr = {
          .fpart.ptr = alloca(100),
          .capacity = 100,
      }
  };
  print_sn(pbuffer, "hello world");
  println("output: ${}", pbuffer.fptr);
}
