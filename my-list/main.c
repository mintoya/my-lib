#include <stdio.h>
#define MY_LIST_C
#include "my-list.h"

int main(void) {

  List *a = mList(int);
  mList_add(a, int, 5);
  mList_insert(a, int, 8, 0);
  printf("%i\n", mList_get(a, int, 0));

  return EXIT_SUCCESS;
}
