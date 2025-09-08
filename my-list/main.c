#include <stdio.h>
#define MY_LIST_C
#include "my-list.h"

typedef struct {
  int a;
  char b;
} typed;
int main(void) {
  List *l = mList(typed, {.a = 5, .b = 'c'}, {.a = 8, .b = 'c'});
  // List *l = mList(int);

  mList_forEach(l, typed, t, { printf("%i\n", t.a); });
  return EXIT_SUCCESS;
}
