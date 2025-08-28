#include "my-list/my-list.h"
#include "umap-printer/printer.h"
#include "umap/umap.h"
#include <stdio.h>
#include <string.h>


int main(void) {

  // delimiters:
  // : -> follows name
  // , -> separates lists
  // { -> starts internal umap
  // } -> ends internal umap
  // [ -> starts internal list
  // ] -> ends internal list

  char *string = "main : { key : [ hello world, ] otherKey : thing }";

  //setting the key "hello" to the string "world"
  UMap* map = UMap_new();
  UMap_set(map, (um_fp){.length = strlen("hello"),.ptr = "hello"}, (um_fp){.length = strlen("world"),.ptr = "world"});
  //setting an index 1 to the string "testString"
  UMap_set(map, (um_fp){.length = sizeof(unsigned int ),.ptr = (int[1]){1}}, (um_fp){.length = strlen("world"),.ptr = "world"});

  return 0;
}
