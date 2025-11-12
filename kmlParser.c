#include "kml.h"
#include "macroList.h"
#include "my-list.h"
#include "print.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wheels.h"

um_fp read_stdin() {
  List *fl = List_new(&defaultAllocator, sizeof(char));
  MList_DF(chars, char, fl);
  int c;

  while ((c = fgetc(stdin)) != EOF) {
    MList_push(chars, c);
  }

  um_fp res = {.ptr = fl->head, .width = sizeof(char) * fl->length};
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
