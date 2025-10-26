#pragma once
#include "../my-list/my-list.h"
#include "../string-List/stringList.h"
#include "../string-List/um_fp.h"
#include "variadic.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

// you shouldnt call print functions besides put
// get around this with
// USENAMEDPRINTER(typenamestring,value) or
// USETYPEPRINTER(type,value)

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
#define GETTYPEPRINTERFN(T) _##T##_printer

#define REGISTER_PRINTER(T, ...)                                               \
  void _##T##_printer(outputFunction put, void *_v_in_ptr) {                   \
    T in = *(T *)_v_in_ptr;                                                    \
    __VA_ARGS__                                                                \
  }                                                                            \
  __attribute__((constructor(201))) static void register_##T() {               \
    stringList_append(typeNamesList,                                           \
                      (um_fp){.ptr = (uint8_t *)#T, .width = sizeof(#T) - 1}); \
    List_append(printerFunctions, (printerFunction[1]){GETTYPEPRINTERFN(T)});  \
  }

#define MERGE_PRINTER_M(a, b) a##b
#define LABEL_PRINTER_GEN(l, a) MERGE_PRINTER_M(l, a)

#define UNIQUE_GEN_PRINTER                                                     \
  LABEL_PRINTER_GEN(LABEL_PRINTER_GEN(__LINE__, _), __COUNTER__)
#define UNIQUE_PRINTER_FN LABEL_PRINTER_GEN(PRINTERFN, UNIQUE_GEN_PRINTER)
#define UNIQUE_PRINTER_FN2                                                     \
  LABEL_PRINTER_GEN(printerConstructor, UNIQUE_GEN_PRINTER)
#define REGISTER_SPECIAL_PRINTER_NEEDID(id, str, type, ...)                    \
  void id(outputFunction put, void *_v_in_ptr) {                               \
    type in = *(type *)_v_in_ptr;                                              \
    __VA_ARGS__                                                                \
  }                                                                            \
  __attribute__((constructor(201))) static void UNIQUE_PRINTER_FN2() {         \
    stringList_append(typeNamesList,                                           \
                      (um_fp){.ptr = (uint8_t *)str, .width = strlen(str)});   \
    List_append(printerFunctions, (printerFunction[1]){id});                   \
  }

#define REGISTER_SPECIAL_PRINTER(str, type, ...)                               \
  REGISTER_SPECIAL_PRINTER_NEEDID(UNIQUE_PRINTER_FN, str, type, __VA_ARGS__)
// for use inside of a registerprinter
// helps use builtin printers
#define USETYPEPRINTER(T, v) GETTYPEPRINTERFN(T)(put, &(v))

#define USENAMEDPRINTER(strname, val)                                          \
  print_f_helper((struct print_arg){.ref = &val, .name = nullUmf},             \
                 um_from(strname), put);
#define um_charArr(um) ((char *)(um.ptr))
struct print_arg {
  void *ref;
  um_fp name;
};
REGISTER_PRINTER(char, { put(&in, 1); });
REGISTER_PRINTER(um_fp, {
  if (in.ptr) {
    put((char *)in.ptr, in.width);
  } else {
    put("__NULLUMF__", 11);
  }
});
REGISTER_PRINTER(int, {
  if (in < 0) {
    put("-", 1);
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
REGISTER_PRINTER(size_t, {
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

// in case of binary data
REGISTER_SPECIAL_PRINTER("um_fp<void>", um_fp, {
  put("0x{", 3);
  while (in.width) {
    uint8_t c = ((uint8_t *)in.ptr)[in.width - 1];
    unsigned char top = c >> 4;
    unsigned char bottom = c & 0x0f;
    // clang-format off
    switch (top) {
    case 0x0: put("0", 1); break;
    case 0x1: put("1", 1); break;
    case 0x2: put("2", 1); break;
    case 0x3: put("3", 1); break;
    case 0x4: put("4", 1); break;
    case 0x5: put("5", 1); break;
    case 0x6: put("6", 1); break;
    case 0x7: put("7", 1); break;
    case 0x8: put("8", 1); break;
    case 0x9: put("9", 1); break;
    case 0xa: put("a", 1); break;
    case 0xb: put("b", 1); break;
    case 0xc: put("c", 1); break;
    case 0xd: put("d", 1); break;
    case 0xe: put("e", 1); break;
    case 0xf: put("f", 1); break;
    }
    switch (bottom) {
    case 0x0: put("0", 1); break;
    case 0x1: put("1", 1); break;
    case 0x2: put("2", 1); break;
    case 0x3: put("3", 1); break;
    case 0x4: put("4", 1); break;
    case 0x5: put("5", 1); break;
    case 0x6: put("6", 1); break;
    case 0x7: put("7", 1); break;
    case 0x8: put("8", 1); break;
    case 0x9: put("9", 1); break;
    case 0xa: put("a", 1); break;
    case 0xb: put("b", 1); break;
    case 0xc: put("c", 1); break;
    case 0xd: put("d", 1); break;
    case 0xe: put("e", 1); break;
    case 0xf: put("f", 1);
      break;
    }
    in.width -= sizeof(uint8_t);
  }
  put("}", 2);

  // clang-format on
});

static void print_f_helper(struct print_arg p, um_fp typeName,
                           outputFunction put) {
  void *ref = p.ref;
  if (!typeName.width) {
    typeName = p.name;
  }
  unsigned int index = stringList_search(typeNamesList, typeName);
  if (index >= printerFunctions->length) {
    GETTYPEPRINTERFN(um_fp)(put, (um_fp[1]){(um_from("_NO_TYPE("))});
    GETTYPEPRINTERFN(um_fp)(put, (um_fp[1]){typeName});
    GETTYPEPRINTERFN(um_fp)(put, (um_fp[1]){(um_from(")"))});
  } else {
    (*((printerFunction *)List_getRef(printerFunctions, index)))(put, ref);
  }
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
        um_fp typeName = {.ptr = ((uint8_t *)fmt.ptr) + i + 1,
                          .width = j - i - 1};
        struct print_arg assumedName = va_arg(l, struct print_arg);
        if (!assumedName.ref)
          return put("__ NO ARGUMENT PROVIDED, ENDING PRINT __", 40);

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

#ifndef __cplusplus
#define MAKE_PRINT_ARG(a)                                                      \
  ((struct print_arg){.ref = ((typeof(a)[1]){a}),                              \
                      .name = _Generic((a),                                    \
                          int: um_from("int"),                                 \
                          char: um_from("char"),                               \
                          um_fp: um_from("um_fp"),                             \
                          size_t: um_from("size_t"),                           \
                          default: nullUmf)})
#else
template <typename T> print_arg make_print_arg(T &a) {
  return print_arg{.ref = &a, .name = nullUmf};
}
template <> print_arg make_print_arg(int &a) {
  return print_arg{.ref = &a, .name = um_from("int")};
}
print_arg make_print_arg(char &a) {
  return print_arg{.ref = &a, .name = um_from("char")};
}
print_arg make_print_arg(size_t &a) {
  return print_arg{.ref = &a, .name = um_from("size_t")};
}
print_arg make_print_arg(um_fp &a) {

  return print_arg{.ref = &a, .name = um_from("um_fp")};
}
#define MAKE_PRINT_ARG(a) make_print_arg(a)
#endif

#define EMPTY_PRINT_ARG ((struct print_arg){.ref = NULL, .name = nullUmf})

#define print(fmt, ...)                                                        \
  print_f(defaultPrinter,                                                      \
          um_from(fmt) __VA_OPT__(, APPLY_N(MAKE_PRINT_ARG, __VA_ARGS__)),     \
          EMPTY_PRINT_ARG)
#define print_wf(fmt, printerfunction, ...)                                    \
  print_f(printerfunction, um_from(fmt),                                       \
          __VA_OPT__(, APPLY_N(MAKE_PRINT_ARG, __VA_ARGS__)), EMPTY_PRINT_ARG)

#define println(fmt, ...) print(fmt "\n", __VA_ARGS__)
#define println_wf(fmt, printerfunction, ...)                                  \
  print_wf(fmt "\n", printerfunction, __VA_ARGS__)
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
