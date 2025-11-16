#define PRINTER_LIST_TYPENAMES
#include "../allocator.h"
#include "../kmlM.h"
#include "../macroList.h"
#include "../my-list.h"
#include "../print.h"
#include "../umap.h"
#include "../wheels.h"
List *buffer = NULL;

__attribute__((destructor)) void freeList(void) { List_free(buffer); }
__attribute__((constructor)) void setupList(void) {
  buffer = List_new(&defaultAllocator, sizeof(char));
}

void listPrinter(const char *c, unsigned int length, char flush) {
  List_appendFromArr(buffer, c, length);
}

static inline fptr um_flist(List *l) {
  return (um_fp){
      .width = List_headArea(l),
      .ptr = l->head,
  };
}
void formatPrinter(const char *c, unsigned int l, char flush) {
  for (int i = 0; i < l; i++)
    switch (c[i]) {
    case ';':
    case '}':
    case '{':
    case '[':
    case ']':
      fwrite(c + i, sizeof(char), 1, stdout);
      fwrite("\n", sizeof(char), 1, stdout);
      break;
    default:
      fwrite(c + i, sizeof(char), 1, stdout);
    }
}
outputFunction formatter = formatPrinter;
int main() {
  UMap_scoped *test = UMap_new(&defaultAllocator);
  UMap_set(test, fp_from("a"), fp_from("b"));
  UMap_set(test, fp_from("b"), fp_from("c"));
  UMap_set(test, fp_from("c"), fp_from("a"));
  UMapList_scoped *testList = UMapList_new(&defaultAllocator);
  UMapList_setChild(testList, 0, test);
  UMapList_append(testList, fp_from("c"));
  UMapList_append(testList, fp_from("d"));
  UMap_setChild(test, fp_from("innermap"), test);
  UMap_setChild(test, fp_from("innermap"), test); // doubley nested
  UMap_setList(test, fp_from("innerlist"), testList);

  UMapView viewp = ((UMapView){UMapList_get(testList, 0)});
  print_wf(listPrinter, "${UMap}", *test);
  UMap_scoped *output = parse(NULL, NULL, um_flist(buffer));
  println("object output : ${UMap}", *output);
}
