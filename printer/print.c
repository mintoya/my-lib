#include "print.h"
#include <stdio.h>
#define STRING_LIST_C
#include "../string-List/stringList.h"
#define MY_LIST_C
#include "../my-list/my-list.h"

REGISTER_PRINTER(char, { put(&in, 1); });
REGISTER_PRINTER(int, {
  if (in < 0) {
    put("0", 1);
    in = -in;
  }
  int l = 1;
  while (l <= in / 10)
    l *= 10;
  while (l) {
    char c = in / l + '0';
    put(&c, 1);
    in %= l;
    l /= 10;
  }
});
REGISTER_PRINTER(um_fp, { put(in.ptr, in.width); });

int main(void) {
  int i = 6;
  print("${int}${int} ${char} ${um_fp}", i, 7, 'c', um_from("hello world"));

  return 0;
}
