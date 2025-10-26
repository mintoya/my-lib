#include "string-List/stringList.h"
#include "string-List/um_fp.h"
// #define PRINTER_LIST_TYPENAMES
#include "printer/print.h"
#include "umap/umap.h"
#define STRING_LIST_C
#include "string-List/stringList.h"
#define MY_LIST_C
#include "my-list/my-list.h"

REGISTER_PRINTER(stringListView, {
  put("[", 1);
  int length = stringListView_MetaList(in).length;
  for (int i = 0; i < length - 1; i++) {
    um_fp um = stringListView_get(in, i);
    USETYPEPRINTER(um_fp, um);
    put("|", 1);
  }
  if (length > 0) {
    um_fp um = stringListView_get(in, length - 1);
    USETYPEPRINTER(um_fp, um);
  }
  put("]", 1);
});
REGISTER_PRINTER(stringList, {
  put("[", 1);
  int length = stringList_length(&in);
  for (int i = 0; i < length - 1; i++) {
    um_fp um = stringList_get(&in, i);
    USETYPEPRINTER(um_fp, um);
    put("|", 1);
  }
  if (length > 0) {
    um_fp um = stringList_get(&in, length - 1);
    USETYPEPRINTER(um_fp, um);
  }
  put("]", 1);
});
REGISTER_SPECIAL_PRINTER("stringList*", stringList *,
                         { USENAMEDPRINTER("stringList", *in); });

REGISTER_PRINTER(UMapView, {
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
REGISTER_PRINTER(UMap, {
  put("{", 1);
  for (int i = 0; i < stringList_length(in.keys); i++) {
    um_fp key = stringList_get(in.keys, i);
    um_fp val = stringList_get(in.vals, i);
    UMap_innertype tn = UMap_getInnerType(&in, i);
    USETYPEPRINTER(um_fp, key);
    stringListView slv;
    List slvt;
    UMapView mv;
    put(":", 2);
    switch (tn) {
    case NORMAL:
      USETYPEPRINTER(um_fp, val);
      break;
    case LIST:
      mv = (UMapView){.raw = val};
      USENAMEDPRINTER("UMapView<int,obj>", mv);
      break;
    case MAP:
      mv = (UMapView){.raw = val};
      USETYPEPRINTER(UMapView, mv);
      break;
    }
    put(";", 1);
  }
  put("}", 1);
});
REGISTER_SPECIAL_PRINTER("UMap*", UMap *, { USENAMEDPRINTER("UMap", *in); });

REGISTER_SPECIAL_PRINTER("List*", List *, {
  put("[", 1);
  for (int i = 0; i < in->length - 1; i++) {
    um_fp ref = {
        .ptr = (uint8_t *)List_getRef(in, i),
        .width = sizeof(int),
    };
    USENAMEDPRINTER("um_fp<void>", ref);
    put(",", 1);
  }

  if (in->length > 0) {
    um_fp ref = {
        .ptr = (uint8_t *)List_getRef(in, in->length - 1),
        .width = sizeof(int),
    };
    USENAMEDPRINTER("um_fp<void>", ref);
  }
  put("]", 1);
});

REGISTER_SPECIAL_PRINTER("UMapView<int,obj>", UMapView, {
  stringListView slv = UMapView_getVals(in);
  List metaList = UMapView_getMeta(in);
  UMap_innertype *meta = (UMap_innertype *)metaList.head;
  put("[", 1);
  for (int i = 0; i < metaList.length; i++) {
    um_fp val = stringListView_get(slv, i);
    switch (meta[i]) {
    case NORMAL:
      USETYPEPRINTER(um_fp, val);
      break;
    case LIST:
      USENAMEDPRINTER("UMapView<int,obj>", (UMapView){.raw = val})
      break;
    case MAP:
      USETYPEPRINTER(UMapView, (UMapView){.raw = val});
      break;
    }
    if (i != metaList.length - 1) {
      put(",", 1);
    }
  }

  put("]", 1);
});
int main() {
  UMap *m = UMap_new();
  UMap_set(m, um_from("hello0"), um_from("world"));
  UMap_set(m, um_from("hello1"), um_from("world1"));
  UMap_setChild(m, um_from("a"), MAP, m);
  UMap_setChild(m, um_from("listTest"), LIST, m);

  UMapView v = {.raw = UMap_getValAtKey(m, um_from("a"))};

  um_fp temp = UMapView_getValAtKey(v, um_from("hello0"));
  println("serchtest, hello0: ${um_fp}", temp);
  temp = UMapView_getValAtKey(v, um_from("emptykey"));
  println("serchtest, emptykey: ${um_fp}", temp);

  println("${UMap*}", m);
  UMapView uv = UMap_toBuf(m);
  println("size: ${} ${um_fp<void>}", uv.raw.width, uv.raw);
  println("size: ${} ${um_fp<void>: c0 }", uv.raw.width, uv.raw);
  println("test");
  UMapView_free(uv);

  UMap_free(m);
  return 0;
}
