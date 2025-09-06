#include "alloc.h"
#include "umap-printer/printer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define LIST_5d3bd19_C
#include "my-list/my-list.h"
#define KML_PARSER_C
#include "kml/kml.h"
#define STRING_LIST_C
#include "string-List/stringList.h"

um_fp read_stdin() {
  List fl = {
      .width = sizeof(char),
      .head = malloc(sizeof(char)),
      .length = 0,
      .size = 1,
  };
  int c;

  for (c = fgetc(stdin); c != EOF; c = fgetc(stdin)) {
    mList_add(&fl, c);
  }

  um_fp res = {.ptr = (&fl)->head, .width = (&fl)->length * (&fl)->width};
  return res;
}
void print_um_fp(const char *label, um_fp fp) {
  printf("%s: ptr=%p, width=%zu\n", label, fp.ptr, fp.width);

  unsigned char *bytes = (unsigned char *)fp.ptr;
  printf("  bytes: ");
  for (size_t i = 0; i < fp.width; i++) {
    printf("%02X ", bytes[i]);
  }
  printf("\n");

  printf("  as string: \"");
  for (size_t i = 0; i < fp.width; i++) {
    unsigned char c = bytes[i];
    if (c >= 32 && c < 127) { // printable ASCII
      putchar(c);
    } else {
      printf("\\x%02X", c);
    }
  }
  printf("\"\n");
}

int main(int narg, char *args[]) {
  printf("\n");
  um_fp res = read_stdin();
  // usePrintln(um_fp, res);

  // usePrintln(um_fp,find(res,fKey("layers")));
  um_fp result = res;

  for (int i = 1; i < narg; i++) {
    if (isdigit(args[i][0])) {
      result = find(result, fIndex(strtol(args[i], NULL, 10)));
    } else {
      result = find(result, fKey(args[i]));
    }
  }
  usePrint(um_fp, result);

  freAllocate(res.ptr);
  // usePrintln(um_fp, find(kml, fKey("tapDances"), fIndex(0), fIndex(2)));
  return EXIT_SUCCESS;
}
