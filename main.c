// #define PRINTER_LIST_TYPENAMES

#include "hmap.h"
#include "print.h"
#include "um_fp.h"
#include "wheels.h"

int main() {
  println("starting");
  HMap *hm = HMap_new();
  println("created");
  HMap_set(hm, um_from("hello"), um_from("world"));
  println("inserted");
  print("${um_fp}", HMap_get(hm, um_from("hello")));
}
