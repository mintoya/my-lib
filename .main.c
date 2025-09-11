#include "umap-printer/printer.h"
#include "umap/umap.h"
#define STRING_LIST_C
#include "string-List/stringList.h"
#define MY_LIST_C
#include "my-list/my-list.h"

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
  usePrint(UMap *, m);
  mPrint("UMap<string>", &m);
  UMap_free(m);
  return 0;
}
