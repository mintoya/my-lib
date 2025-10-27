#include "string-List/stringList.h"
#include "string-List/um_fp.h"
// #define PRINTER_LIST_TYPENAMES
#include "printer/print.h"
#include "umap/umap.h"

#define PRINTER_C
#include "printer/print.h"
#define STRING_LIST_C
#include "string-List/stringList.h"
#define MY_LIST_C
#include "my-list/my-list.h"

int main() {
  UMap *m = UMap_new();
  UMap_set(m, um_from("hello0"), um_from("world"));
  UMap_set(m, um_from("hello1"), um_from("world1"));
  UMap_setChild(m, um_from("a"), MAP, m);

  UMap *listTest = UMap_new();
  UMap_setIndex(listTest, '5', um_from("fifth"));
  UMap_setChildIndex(listTest, '0', MAP, m);

  println("${UMap*}", listTest);
  println();

  UMap_setChild(m, um_from("listTest"), LIST, m);

  UMapView v = {.raw = UMap_getValAtKey(m, um_from("a"))};

  um_fp temp = UMapView_getValAtKey(v, um_from("hello0"));
  println("serchtest, hello0: ${um_fp}", temp);
  println();
  temp = UMapView_getValAtKey(v, um_from("emptykey"));
  println("serchtest, emptykey: ${um_fp}", temp);
  println();
  // println("${UMap*}", m);
  println();
  UMapView uv = UMap_toBuf(m);
  println("${um_fp<void>}", uv.raw);
  println();
  println("${um_fp<void>:c0 length}", uv.raw);
  println();
  UMapView_free(uv);

  UMap_free(listTest);
  UMap_free(m);
  // stdout_printer();
  return 0;
}
