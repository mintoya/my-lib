#include "printer/print.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "string-List/um_fp.h"
#define KML_PARSER_C
#include "kml/kml.h"
#define STRING_LIST_C
#include "string-List/stringList.h"
#define MY_LIST_C
#include "my-list/my-list.h"

um_fp read_stdin() {
  List fl = {
      .width = sizeof(char),
      .head = malloc(sizeof(char) * 10),
      .length = 0,
      .size = 10,
  };
  int c;

  while ((c = fgetc(stdin)) != EOF) {
    mList_add(&fl, char, c);
  }

  um_fp res = {.ptr = (&fl)->head, .width = (&fl)->length * (&fl)->width};
  return res;
}


int main(int narg, char *args[]) {
  um_fp res = read_stdin();
  um_fp result = res;

  for (int i = 1; i < narg; i++) {
    if (isdigit(args[i][0])) {
      result = find(result, fIndex(strtol(args[i], NULL, 10)));
    } else {
      result = find(result, fKey(args[i]));
    }
  }
  print("${}", result);

  freAllocate(res.ptr);
  return EXIT_SUCCESS;
}
