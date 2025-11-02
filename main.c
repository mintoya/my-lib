// #define PRINTER_LIST_TYPENAMES

#include "hmap.h"
#include "kmlM.h"
#include "my-list.h"
#include "print.h"
#include "um_fp.h"
#include "umap.h"
#include "wheels.h"

int main() {
  // UMap_scoped *test = UMap_new();
  // UMap_set(test, um_from("a"), um_from("b"));
  // UMap_set(test, um_from("b"), um_from("b"));
  // UMap_set(test, um_from("c"), um_from("b"));
  // UMapList_scoped *testList = UMapList_new();
  // UMapList_setChild(testList, 0, test);
  // UMapList_set(testList, 1, um_from("c"));
  // UMap_setChild(test, um_from("innermap"), test);
  // UMap_setList(test, um_from("innermap"), testList);
  // print("${UMap*}", test);
  // UMapView inner = (UMapView){UMap_get(output, um_from("a"))};
  // println("${}", UMapView_getValAtKey(inner, um_from("b")));
  UMap_scoped *output =
      parse(NULL, NULL, um_from("a:{b:\"d;\";;;};test:[{key:value},5]"));
  println("${UMap*}", output);
}
