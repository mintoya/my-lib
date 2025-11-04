#include "kml.h"
#include "my-list.h"
#include "print.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wheels.h"

um_fp read_stdin() {
  List fl = {
      .width = sizeof(char),
      .length = 0,
      .size = 10,
      .head = (uint8_t *)malloc(sizeof(char) * 10),
  };
  int c;

  while ((c = fgetc(stdin)) != EOF) {
    mList_add(&fl, char, c);
  }

  um_fp res = {.ptr = (&fl)->head, .width = (&fl)->length * (&fl)->width};
  return res;
}

#define isdigit(c) ((c <= '9') ? (c >= '0') : (0))
int main(int narg, char *args[]) {
  um_fp res = read_stdin();
  um_fp result = res;

  for (int i = 1; i < narg; i++) {
    if (isdigit(args[i][0])) {
      int index = strtol(args[i], NULL, 10);
      result = findIndex(result, index);
      println("finding index ${}", index);
    } else {
      um_fp key = ((um_fp){.ptr = (uint8_t *)(args[i]),
                           .width = (size_t)strlen(args[i])});
      result = findKey(result, key);
      println("finding key ${um_fp}, length: ${}", key, key.width);
    }
  }
  print("${}", result);

  free(res.ptr);

  return EXIT_SUCCESS;
}
