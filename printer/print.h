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

#define GETTYPEFN(T) _##T##_printer
#define REGISTER_PRINTER(T, expr)                                              \
  void _##T##_printer(outputFunction put, void *_v_in_ptr) {                   \
    T in = *(T *)_v_in_ptr;                                                    \
    expr                                                                       \
  }                                                                            \
  __attribute__((constructor(201))) static void register_##T() {               \
    stringList_append(typeNamesList,                                           \
                      (um_fp){.ptr = (void *)#T, .width = sizeof(#T) - 1});    \
    List_append(printerFunctions, (printerFunction[1]){GETTYPEFN(T)});         \
  }
#define MERGE_(a, b) a##b
#define LABEL_(l, a) MERGE_(l, a)
#define UNIQUE_FN LABEL_(PRINTERFN, __LINE__)
#define UNIQUE_FN2 LABEL_(PRINTERFN2, __LINE__)
#define REGISTER_SPECIAL_PRINTER(str, type, expr)                              \
  void UNIQUE_FN(outputFunction put, void *_v_in_ptr) {                        \
    type in = *(type *)_v_in_ptr;                                              \
    expr                                                                       \
  }                                                                            \
  __attribute__((constructor(201))) static void UNIQUE_FN2() {                 \
    stringList_append(typeNamesList,                                           \
                      (um_fp){.ptr = (void *)str, .width = strlen(str)});      \
    List_append(printerFunctions, (printerFunction[1]){UNIQUE_FN});            \
  }

// for use inside of a registerprinter
// helps use builtin printers
#define USETYPEPRINTER(T, v) GETTYPEFN(T)(put, &(v))

#define um_charArr(um) ((char *)(um.ptr))

struct print_arg {
  void *ref;
  um_fp name;
};

REGISTER_PRINTER(um_fp, { put((char *)in.ptr, in.width); });
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
REGISTER_PRINTER(char, { put(&in, 1); });
static void print_f_helper(struct print_arg p, um_fp typeName,
                           outputFunction put) {
  void *ref = p.ref;
  if (!typeName.width) {
    typeName = p.name;
  }
  unsigned int index = stringList_search(typeNamesList, typeName);
  if (index == printerFunctions->length) {
    GETTYPEFN(um_fp)(put, (um_fp[1]){(um_from("_NO_TYPE("))});
    GETTYPEFN(um_fp)(put, (um_fp[1]){typeName});
    GETTYPEFN(um_fp)(put, (um_fp[1]){(um_from(")"))});
  }
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
        struct print_arg assumedName = va_arg(l, struct print_arg);
        print_f_helper(assumedName, typeName, put);
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

#define ASSUME_TYPE(a)                                                         \
  _Generic((a),                                                                \
      int: ((um_fp){.ptr = "int", .width = 3}),                                \
      char: ((um_fp){.ptr = "char", .width = 4}),                              \
      um_fp: ((um_fp){.ptr = "um_fp", .width = 5}),                            \
      default: ((um_fp){.ptr = NULL, .width = 0}))

#define MAKE_REFERANCE(a) ((typeof(a)[1]){a})
#define MAKE_PRINT_ARG(a)                                                      \
  ((struct print_arg){.ref = MAKE_REFERANCE(a), .name = ASSUME_TYPE(a)})
#define print(fmt, ...)                                                        \
  print_f(defaultPrinter, um_from(fmt), APPLY_N(MAKE_PRINT_ARG, __VA_ARGS__))
#define print_wf(fmt, printerfunction, ...)                                    \
  print_f(printerfunction, um_from(fmt), APPLY_N(MAKE_PRINT_ARG, __VA_ARGS__))
static void defaultPrinter(char *c, unsigned int length) {
  fwrite(c, sizeof(char), length, stdout);
}
#ifdef PRINTER_LIST_TYPENAMES
__attribute__((constructor(203))) static void post_init() {
  _um_fp_printer(defaultPrinter, (um_fp[1]){um_from("list of type names:\n")});
  for (int i = 0; i < stringList_length(typeNamesList); i++) {
    _um_fp_printer(defaultPrinter, (um_fp[1]){um_from("  ")});
    _um_fp_printer(defaultPrinter,
                   (um_fp[1]){stringList_get(typeNamesList, i)});
    _um_fp_printer(defaultPrinter, (um_fp[1]){um_from("\n")});
  }
}
#endif
#undef um_charArr
