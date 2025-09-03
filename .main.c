#include "string-List/stringList.h"
#include "umap-printer/printer.h"
#include "umap/umap.h"

int main()
{
  UMap* m = UMap_new();
  UMap_set(m,um_from("hello0"),um_from("world"));
  UMap_set(m,um_from("hello1"),um_from("world"));
  UMap_set(m,um_from("hello2"),um_from("world"));
  UMap_set(m,um_from("hello3"),um_from("world"));
  UMap_set(m,um_from("hello0"),um_from("world"));
  UMap_set(m,um_from("hello2"),um_from("world"));
  UMap_set(m,um_from("hello2"),um_from("world"));
  UMap_set(m,um_from("hello0"),um_from("world"));
  mPrint("UMap<string>", &m);
  return 0;
}
