#include "../string-List/um_fp.h"
#include "print.h"
#include "variadic.h"
#include <stdint.h>
#define STRING_LIST_C
#include "../string-List/stringList.h"
#define MY_LIST_C
#include "../my-list/my-list.h"

// only char,int,and um_fp are registered by default
// those can be ascessed by USETYPEPRINTER inside a REGISTER_PRINTER

REGISTER_PRINTER(List, {
  for (int i = 0; i < in.length; i++) {
    um_fp res;
    res.ptr = List_getRef(&in, i);
    res.width = in.width;
    USENAMEDPRINTER("um_fp<void>", res);
  }
});
REGISTER_SPECIAL_PRINTER("char*", char *, {
  int i;
  for (i = 0; in[i]; i++)
    ;
  USETYPEPRINTER(um_fp, ((um_fp){.ptr = in, .width = i}));
});
typedef struct test {
  char c;
  int i;
  uint8_t u;
  uint8_t u2;
} testStruct;

REGISTER_PRINTER(testStruct, {
  put("{", 1);
  USETYPEPRINTER(int, in.i);
  put(",", 1);
  USETYPEPRINTER(char, in.c);
  put("}", 1);
});

int main(void) {
  testStruct ts = {.c = 'c', .i = -1, .u = 0x67, .u2 = 0x41};
  // println();
  // COUNT_ARGS( );

  println("${}", 1, 2, 3, 4, 5, 6, 7);
  println("special printer ${testStruct}", ts);
  println("special printer ${um_fp<void>}",
          ((um_fp){.ptr = &ts, .width = sizeof(ts)}));
  List *l = mList(int, 1, 2, 4);
  println("${List}", *l);
  return 0;
}
