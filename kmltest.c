#include "kml.h"

#include "hmap.h"
#include "kmlM.h"
#include "macroList.h"
#include "my-list.h"
#include "print.h"
#include "um_fp.h"
#include "umap.h"
#include "wheels.h"

List *buffer = NULL;
__attribute__((destructor)) void freeList(void) { List_free(buffer); }
__attribute__((constructor)) void setupList(void) { buffer = mList(char); }
void listPrinter(char *c, unsigned int length) {
  List_appendFromArr(buffer, c, length);
}

int main() {
  UMap_scoped *test = UMap_new();
  UMap_set(test, um_from("a"), um_from("a"));
  UMap_set(test, um_from("b"), um_from("b"));
  UMap_set(test, um_from("c"), um_from("c"));
  UMapList_scoped *testList = UMapList_new();
  UMapList_setChild(testList, 0, test);
  UMapList_append(testList, um_from("c"));
  UMap_setChild(test, um_from("innermap"), test);
  UMap_setList(test, um_from("innerlist"), testList);

  print_wf(listPrinter, "${UMap*}", test);
  um_fp listBuffer =
      ((um_fp){.ptr = buffer->head, .width = List_headArea(buffer)});

  println("string output: ${}", listBuffer);
  UMap_scoped *output = parse(NULL, NULL, listBuffer);
  println("entire object: ${UMap*}", output);
}
