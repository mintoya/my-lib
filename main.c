#include "string-List/stringList.h"
#include "string-List/um_fp.h"
// #define PRINTER_LIST_TYPENAMES
#include "printer/print.h"
#include "umap/umap.h"
#define STRING_LIST_C
#include "string-List/stringList.h"
#define MY_LIST_C
#include "my-list/my-list.h"

REGISTER_SPECIAL_PRINTER("UMapView", UMapView, {
  stringListView keys = UMapView_getKeys(in);
  stringListView vals = UMapView_getVals(in);
  List metaList = UMapView_getMeta(in);
  UMap_innertype *meta = (UMap_innertype *)metaList.head;

  put("{", 1);
  for (int i = 0; i < stringListView_MetaList(keys).length; i++) {
    um_fp key = stringListView_get(keys, i);
    um_fp val = stringListView_get(vals, i);
    UMap_innertype tn = meta[i];
    USETYPEPRINTER(um_fp, key);
    put(":", 2);
    switch (tn) {
    case NORMAL:
      USETYPEPRINTER(um_fp, val);
      break;
    case LIST:
      USETYPEPRINTER(stringListView, UMapView_getVals((UMapView){.raw = val}));
      break;
    case MAP:
      USETYPEPRINTER(UMapView, (UMapView){.raw = val});
      break;
    }
    put(";", 1);
  }
  put("}", 1);
});
// takes type as an arg
// int assumes its a list
// um_fp assumes its a map
REGISTER_SPECIAL_PRINTER("UMap*", UMap *, {
  enum maporlist { map = 0, list } objtype = (enum maporlist){map};
  PRINTERARGSEACH({
    if (um_eq(arg, um_from("um_fp"))) {
      objtype = map;
    }
    if (um_eq(arg, um_from("int"))) {
      objtype = list;
    }
  });
  put("{", 1);
  UMap_innertype *meta = (UMap_innertype *)in->metadata->head;
  switch (objtype) {
  case map: {
    stringList *vals = in->vals;
    stringList *keys = in->keys;
    for (int i = 0; i < stringList_length(keys); i++) {
      USETYPEPRINTER(um_fp, stringList_get(keys, i));
      put(":", 1);
      switch (meta[i]) {
      case NORMAL:
        USETYPEPRINTER(um_fp, stringList_get(vals, i));
        break;
      case LIST:
        USENAMEDPRINTER("UMapView", (UMapView){.raw = stringList_get(vals, i)});
        break;
      case MAP:
        USENAMEDPRINTER("UMapView", (UMapView){.raw = stringList_get(vals, i)});
        break;
      }
      put(";", 1);
    }
  } break;
  case list: {
    stringList *vals = in->vals;
    stringList *keys = in->keys;
    if (stringList_length(keys)) {
      int max = *(int *)stringList_get(keys, 0).ptr;
      for (int i = 0; i < stringList_length(vals); i++) {
        int current = *(int *)stringList_get(keys, i).ptr;
        max = (max > current) ? (max) : (current);
      }
      for (int i = 0; i < max; i++) {
        USETYPEPRINTER(um_fp, stringList_get(vals, i));
        put(",", 1);
      }
    }
  } break;
  }
  put("}", 1);
});

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
  println("${UMap*}", m);
  println();
  UMapView uv = UMap_toBuf(m);
  println("${um_fp<void>:length}", uv.raw);
  println();
  println("${um_fp<void>:c0 length}", uv.raw);
  println();
  UMapView_free(uv);

  UMap_free(listTest);
  UMap_free(m);
  return 0;
}
