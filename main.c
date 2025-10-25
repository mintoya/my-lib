#include "string-List/um_fp.h"
#define PRINTER_LIST_TYPENAMES
#include "printer/print.h"
#include "umap/umap.h"
#define STRING_LIST_C
#include "string-List/stringList.h"
#define MY_LIST_C
#include "my-list/my-list.h"

REGISTER_PRINTER(stringList, {
  for (int i = 0; i < stringList_length(&in); i++) {
    put("{", 1);
    um_fp um = stringList_get(&in, i);
    USETYPEPRINTER(um_fp, um);
    put("}", 1);
  }
})
REGISTER_SPECIAL_PRINTER("stringList*", stringList *,
                         { USENAMEDPRINTER("stringList", *in); });
REGISTER_PRINTER(UMap, {
  for (int i = 0; i < stringList_length(in.keys); i++) {
    put("{", 1);
    um_fp key = stringList_get(in.keys, i);
    um_fp val = stringList_get(in.vals, i);
    USETYPEPRINTER(um_fp, key);
    put("->", 2);
    USETYPEPRINTER(um_fp, val);
    put("}", 1);
  }
});
REGISTER_SPECIAL_PRINTER("UMap*", UMap *, { USENAMEDPRINTER("UMap", *in); });
int main() {
  UMap *m = UMap_new();
  UMap_set(m, um_from("hello0"), um_from("world"));
  UMap_set(m, um_from("hello1"), um_from("world"));
  UMap_set(m, um_from("hello2"), um_from("world"));
  UMap_set(m, um_from("hello3"), um_from("world"));
  UMap_set(m, um_from("hello0"), um_from("world"));
  UMap_set(m, um_from("hello2"), um_from("world"));
  UMap_set(m, um_from("hello2"), um_from("world"));
  UMap_set(m, um_from("hello0"), um_from("world"));
  UMap_set(m, um_from("hello2"), nullUmf);

  println("${stringList*}", m->keys);

  println("${UMap*}", m);
  UMap *map2 = UMap_remake(m);
  println("${stringList*}", m->keys);

  println("${stringList*}", map2->keys);

  println("${UMap*}", map2);

  UMap_free(m);
  UMap_free(map2);
  return 0;
}
