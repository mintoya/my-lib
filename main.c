// #define PRINTER_LIST_TYPENAMES

#include "hmap.h"
#include "print.h"
#include "um_fp.h"
#include "wheels.h"

int main() {
  HMap_scoped *hm = HMap_new();
  HMap_set(hm, um_from("hello"), um_from("world"));
  HMap_set(hm, um_from("hello2"), um_from("world2"));
  HMap_set(hm, um_from("hello3"), um_from("world1"));
  println("${um_fp}", HMap_get(hm, um_from("hello")));
  println("${um_fp}", HMap_get(hm, um_from("hello2")));
  println("${um_fp}", HMap_get(hm, um_from("hello3")));
}
