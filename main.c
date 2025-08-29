#include "my-list/my-list.h"
#include "umap-printer/printer.h"
#include "umap/umap.h"
#include <stdio.h>
#include <string.h>

char *delims = ":,{}[]; ";
char *val = ":;";
char *lst = "[]";
char *sep = ",";
char *obj = "{}";
// delimiters:
// : -> follows name
// ; -> ends defenition
// , -> separates lists
// { -> starts internal umap
// } -> ends internal umap
// [ -> starts internal list
// ] -> ends internal list
/*
  objects:
  name:<string/list/object>;
  lists:
  name:[<string/list/object>,<string/list/object>...];
 */

um_fp inside(char limits[2], um_fp string) {

  char front = limits[0];
  char back = limits[1];

  for (int i = 0; i < string.length; i++) {
    if (((char *)string.ptr)[i] == front) {
      unsigned int counter = 1;
      for (int ii = 1; ii + i < string.length; ii++) {
        if (((char *)string.ptr)[i + ii] == front) {
          counter++;
        } else if (((char *)string.ptr)[i + ii] == back) {
          counter--;
        }
        if (!counter)
          return (um_fp){.ptr = ((char *)string.ptr) + i + 1, .length = ii - 1};
      }
    }
  }
  return nullUmf;
}
um_fp until(char delim, um_fp string) {
  int i = 0;
  while (i < string.length && ((char *)string.ptr)[i] != delim) {
    i++;
  }
  string.length = i;
  return string;
}

char str[] = {
#embed "test.txt"
};

int main(void) {

  char *testString = str;

  um_fp str = um_from(testString);
  usePrint(um_fp, str);
  usePrint(char*, "inside");
  usePrint(um_fp, inside("[,", str));
  usePrint(char*, "until :");
  usePrint(um_fp, until(':', str));
  return 0;
}
