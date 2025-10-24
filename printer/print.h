#pragma once
#include "../my-list/my-list.h"
#include "../string-List/stringList.h"
#include "../string-List/um_fp.h"
#include "variadic.h"
#include <stdarg.h>
#include <stdio.h>
typedef void (*outputFunction)(char *, unsigned int length);
typedef void (*printerFunction)(outputFunction, void *);

static List *printerFunctions;
static stringList *typeNamesList;

__attribute__((constructor(200))) static void printerInit() {
  printerFunctions = List_new(sizeof(printerFunction));
  typeNamesList = stringList_new();
}
__attribute__((destructor)) static void printerDeinit() {
  List_free(printerFunctions);
  stringList_free(typeNamesList);
}

#define REGISTER_PRINTER(T, expr)                                              \
  void _##T##_printer(outputFunction put, void *_v_in_ptr) {                   \
    T in = *(T *)_v_in_ptr;                                                    \
    expr                                                                       \
  }                                                                            \
  __attribute__((constructor(201))) static void register_##T() {               \
    stringList_append(typeNamesList,                                           \
                      (um_fp){.ptr = (void *)#T, .width = sizeof(#T) - 1});    \
    List_append(printerFunctions, (printerFunction[1]){_##T##_printer});       \
  }

#define um_charArr(um) ((char *)(um.ptr))

static void print_f_helper(void *ref, um_fp typeName, outputFunction put) {
  unsigned int index = stringList_search(typeNamesList, typeName);
  if (index == printerFunctions->length)
    return;
  (*((printerFunction *)List_getRef(printerFunctions, index)))(put, ref);
}
static void print_f(outputFunction put, um_fp fmt, ...) {
  va_list l;
  va_start(l, fmt);
  char check = 0;
  for (unsigned int i = 0; i < fmt.width; i++) {
    switch (um_charArr(fmt)[i]) {
    case '$':
      if (check)
        put(um_charArr(fmt) + i - 1, 1);
      if (i + 1 == fmt.width)
        put("$", 1);
      check = 1;
      break;
    case '{':
      if (check) {
        unsigned int j;
        for (j = i + 1; j < fmt.width && um_charArr(fmt)[j] != '}'; j++)
          ;
        um_fp typeName = {.ptr = ((char *)fmt.ptr) + i + 1, .width = j - i - 1};
        print_f_helper(va_arg(l, void *), typeName, put);
        i = j;
        check = 0;
      }
      break;
    default:
      if (check)
        put(um_charArr(fmt) + i - 1, 1);
      put(um_charArr(fmt) + i, 1);
      check = 0;
    }
  }
  va_end(l);
}
#define MAKE_REFERANCE(a) ((typeof(a)[1]){a})
#define print(fmt, ...)                                                        \
  print_f(defaultPrinter, um_from(fmt), APPLY_N(MAKE_REFERANCE, __VA_ARGS__))

static void defaultPrinter(char *c, unsigned int length) {
  fwrite(c, sizeof(char), length, stdout);
}
