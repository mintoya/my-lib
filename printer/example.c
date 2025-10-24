#include "print.h"
#include "variadic.h"
#define STRING_LIST_C
#include "../string-List/stringList.h"
#define MY_LIST_C
#include "../my-list/my-list.h"

// only char,int,and um_fp are registered by default
// those can be ascessed by USETYPEPRINTER inside a REGISTER_PRINTER

REGISTER_SPECIAL_PRINTER("char*", char *, {
  while (*in) {
    put(in, 1);
    in++;
  }
});
typedef struct test {
  int i;
  char c;
} testStruct;
REGISTER_PRINTER(testStruct, {
  put("{", 1);
  USETYPEPRINTER(int, in.i);
  put(",", 1);
  USETYPEPRINTER(char, in.c);
  put("}", 1);
});
int main(void) {

  testStruct ts = {.c = 'c', .i = 1};
  print("${}", 1, 2, 3, 4, 5, 6, 7);

  print("special printer ${testStruct}", ts);
  COUNT_ARGS(1);
  return 0;
}
