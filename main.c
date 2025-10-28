#include "string-List/stringList.h"
#include "string-List/um_fp.h"
// #define PRINTER_LIST_TYPENAMES
#include "printer/print.h"
#include "umap/umap.c"
#include "umap/umap.h"
#define PRINTER_C
#include "printer/print.h"
#define STRING_LIST_C
#include "string-List/stringList.h"
#define MY_LIST_C
#include "my-list/my-list.h"

int main() {
  UMap *m = UMap_new();

  UMapList *l = UMapList_new();
  UMapList_append(l, um_from("first"));
  UMapList_append(l, um_from("second"));
  UMapList_set(l, 0, um_from("first overwrite"));
  UMapList_set(l, 5, um_from("fith far"));

  UMap_set(m, um_from("hello0"), um_from("world"));
  UMap_set(m, um_from("hello1"), um_from("world1"));
  UMap_setChild(m, um_from("deepMap"), m);
  UMap_setList(m, um_from("deepList"), l);

  println("${UMapList*}", l);
  println("${UMap*}", m);

  UMapView umv = UMap_toBuf(m);
  println("${um_fp<void>:c0 length}", umv.raw);

  UMapList_free(l);
  UMap_free(m);
  return 0;
}
