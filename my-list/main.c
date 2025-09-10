#include <stdio.h>
#define MY_LIST_C
#include "my-list.h"

typedef struct {
  int a;
  char b;
} typed;
int main(void) {
  List *l = mList(typed, {.a = 5, .b = 'c'}, {.a = 8, .b = 'c'});
  List_append(l, ((typed[]){{.a = 2, .b = 't'}}));
  mList_add(l, typed, {.a = 5, .b = 'c'});
  List_insert(l, 0, (typed[1]){{.a = 5, .b = 'c'}});
  List_insert(l, 0, (typed[1]){{.a = 5, .b = 'c'}});

  mList_forEach(l, typed, t,
                printf("t:\n"
                       "\t%i\n"
                       "\t%c\n",
                       t.a, t.b););
  return EXIT_SUCCESS;
}
