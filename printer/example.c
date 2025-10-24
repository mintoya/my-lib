#include "print.h"
#define STRING_LIST_C
#include "../string-List/stringList.h"
#define MY_LIST_C
#include "../my-list/my-list.h"

// only char,int,and um_fp are registered by default
// those can be ascessed by USETYPEPRINTER inside a REGISTER_PRINTER

REGISTER_SPECIAL_PRINTER("char*", char *, {
  int i;
  for (i = 0; in[i]; i++)
    ;
  USETYPEPRINTER(um_fp, ((um_fp){.ptr = in, .width = i}));
});
typedef struct test {
  char c;
  int i;
} testStruct;

REGISTER_PRINTER(testStruct, {
  put("{", 1);
  USETYPEPRINTER(int, in.i);
  put(",", 1);
  USETYPEPRINTER(char, in.c);
  put("}", 1);
});

int main(void) {
  testStruct ts = {.c = 'c', .i = -1};
  println("${}", 1, 2, 3, 4, 5, 6, 7);
  println("special printer ${testStruct}", ts);
  println("special printer ${um_fp<void>}",
          ((um_fp){.ptr = &ts, .width = sizeof(ts)}));
  return 0;
}
