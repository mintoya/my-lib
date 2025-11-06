

#include "kmlM.h"
#include "macroList.h"
#include "my-list.h"
#include "print.h"
#include "umap.h"
#include "wheels.h"

List *buffer = NULL;
MList(char) buf;

__attribute__((destructor)) void freeList(void) { List_free(buffer); }
__attribute__((constructor)) void setupList(void) { MList_DFInit(buf, buffer); }

void listPrinter(char *c, unsigned int length, char flush) {
  MList_DF(buf, buffer);
  MList_addArr(buf, length, c);
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
  UMap_scoped *output = parse(NULL, NULL, MList_fp(buf));
  println("object input : ${}", MList_fp(buf));
  println("entire object: ${UMap*}", output);
}
