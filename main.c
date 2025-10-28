// #define PRINTER_LIST_TYPENAMES

#include "print.h"
#include "stringList.h"
#include "um_fp.h"
#include "umap.h"
// sets up headers
#include "wheels.h"

int main() {
  List_scoped *list = mList(int);
  UMap *m = UMap_new();
  UMapList *l = UMapList_new();
  UMapList_append(l, um_from("first"));
  UMapList_append(l, um_from("second"));
  UMapList_set(l, 0, um_from("first overwrite"));
  UMapList_set(l, 5, um_from("fith far"));

  UMap_set(m, um_from("hello0"), um_from("world"));
  UMap_set(m, um_from("hello1"), um_from("world1"));
  UMap_setChild(m, um_from("deepMap"), m);
  UMap_setChild(m, um_from("deepMap"), m);

  UMap_setList(m, um_from("deepList"), l);

  println("${UMapList*}\n${UMap*}\n", l, m);

  UMapView umv = UMap_toBuf(m);
  println("${um_fp<void>:c0 length}", umv.raw);

  UMapList_free(l);
  UMap_free(m);
  return 0;
}
