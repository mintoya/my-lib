#ifndef PRINTER_H
#define PRINTER_H
#include "../my-list/my-list.h"
#include "../string-List/stringList.h"
#include "../string-List/um_fp.h"
#include "variadic.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

// temporary variables for compound literals
#ifdef __cplusplus
// clang-format off
  #include <type_traits>
  #include <utility>
  template <typename T> class StackPush {
  private:
    using RealT = std::remove_reference_t<T>;
    RealT value; // store inline, no heap
  public:
    template <typename ArgType>
    explicit StackPush(ArgType &&arg) : value(std::forward<ArgType>(arg)) {}
    operator RealT &() { return value; }
    operator const RealT &() const { return value; }
    operator void *() { return (void *)&value; }
    operator const void *() const { return (const void *)&value; }
  };
// clang-format on
#define typeof(m) decltype(m)
#define REF(type, value) (StackPush<type>(value))
#else
#define REF(type, value) ((type[1]){value})
#endif

typedef void (*outputFunction)(char *, unsigned int length);
typedef void (*printerFunction)(outputFunction, void *, um_fp args);
// typedef struct {
//   outputFunction o;
//   um_fp env;
// } outputClosure;

static List *printerFunctions;
static stringList *typeNamesList;
// arg utils
// clang-format off

  unsigned int printer_arg_indexOf(um_fp string, char c);
  um_fp printer_arg_until(char delim, um_fp string) ;
  um_fp printer_arg_after(char delim, um_fp slice) ;
  um_fp printer_arg_trim(um_fp in) ;

// clang-format on

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
  __attribute__((weak)) void GETTYPEPRINTERFN(T)(                              \
      outputFunction put, void *_v_in_ptr, um_fp args) {                       \
    (void)args;                                                                \
    T in = *(T *)_v_in_ptr;                                                    \
    __VA_ARGS__                                                                \
  }                                                                            \
  __attribute__((constructor(201)))                                            \
  __attribute__((weak)) void register_##T() {                                  \
    stringList_append(typeNamesList,                                           \
                      (um_fp){.ptr = (uint8_t *)#T, .width = sizeof(#T) - 1}); \
    List_append(printerFunctions, REF(printerFunction, GETTYPEPRINTERFN(T)));  \
  }

#define MERGE_PRINTER_M(a, b) a##b
#define LABEL_PRINTER_GEN(l, a) MERGE_PRINTER_M(l, a)

#define UNIQUE_GEN_PRINTER                                                     \
  LABEL_PRINTER_GEN(LABEL_PRINTER_GEN(__LINE__, _), __COUNTER__)
#define UNIQUE_PRINTER_FN LABEL_PRINTER_GEN(PRINTERFN, UNIQUE_GEN_PRINTER)
#define UNIQUE_PRINTER_FN2                                                     \
  LABEL_PRINTER_GEN(printerConstructor, UNIQUE_GEN_PRINTER)
#define REGISTER_SPECIAL_PRINTER_NEEDID(id, str, type, ...)                    \
  __attribute__((weak)) void id(outputFunction put, void *_v_in_ptr,           \
                                um_fp args) {                                  \
    type in = *(type *)_v_in_ptr;                                              \
    __VA_ARGS__                                                                \
  }                                                                            \
  __attribute__((constructor(201))) static void UNIQUE_PRINTER_FN2() {         \
    stringList_append(typeNamesList,                                           \
                      (um_fp){.ptr = (uint8_t *)str, .width = strlen(str)});   \
    List_append(printerFunctions, REF(printerFunction, id));                   \
  }

#define REGISTER_SPECIAL_PRINTER(str, type, ...)                               \
  REGISTER_SPECIAL_PRINTER_NEEDID(UNIQUE_PRINTER_FN, str, type, __VA_ARGS__)
#define USETYPEPRINTER(T, val) GETTYPEPRINTERFN(T)(put, REF(T, val), nullUmf)
#define USENAMEDPRINTER(strname, val)                                          \
  print_f_helper(                                                              \
      (struct print_arg){.ref = REF(typeof(val), val), .name = nullUmf},       \
      printer_arg_trim(printer_arg_until(':', um_from(strname))), put,         \
      printer_arg_after(':', um_from(strname)));
#define PRINTERARGSEACH(...)                                                   \
  um_fp tempargs = printer_arg_trim(args);                                     \
  while (tempargs.width) {                                                     \
    um_fp arg = printer_arg_until(' ', tempargs);                              \
    __VA_ARGS__                                                                \
    tempargs = printer_arg_after(' ', tempargs);                               \
    tempargs = printer_arg_trim(tempargs);                                     \
  }

struct print_arg {
  void *ref;
  um_fp name;
};
// clang-format off

// examples with builtin types

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wwrite-strings"
#pragma clang diagnostic ignored "-Wheader-hygiene"
#pragma clang diagnostic ignored "-Wundefined-inline"


// the behavior of "put" is modular
// 
// for building printers
// USENAMEDPRINTER(printerid,value)
// USETYPEPRINTER(type,value)
//
// typeprinter skips the search
// named printer skips the string parsing
// use print_wf with put to keep output consistant
// 
// you can pass args with a printerid and a colon 
// ex: "um_fp<void>: c0 length"
// assumptions are not enabled with a c++ compiler,
// so no ${}

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

  REGISTER_SPECIAL_PRINTER("um_fp<void>", um_fp, {

    char cut0s = 0;
    char useLength = 0;
    PRINTERARGSEACH({
      if (um_eq(um_from("c0"), arg)) {
        cut0s = 1;
      } else if (um_eq(um_from("length"), arg)) {
        useLength = 1;
      }
    });


    put("um_fp:", 6);
    if(useLength){
      USETYPEPRINTER(size_t, in.width);
    }else{
    }
    put("{", 1);
    int counter = 0;
    while (in.width) {
      uint8_t c = ((uint8_t *)in.ptr)[in.width - 1];
      unsigned char top = c >> 4;
      unsigned char bottom = c & 0x0f;

      if (!top && !bottom) {
        counter += 2;
      } else if (!bottom) {
        counter++;
      } else if (!top) {
        counter++;

        if (cut0s) {
          put("(", 1);
          USETYPEPRINTER(int, counter);
          put(")", 1);
        } else {
          while(counter){
            counter--;
            put("0", 1);
          }
        }
        counter = 0;
      }

      // clang-format off
      switch (top) {
       case 0x1: put("1", 1); break;
        case 0x2: put("2", 1); break; case 0x3: put("3", 1); break;
        case 0x4: put("4", 1); break; case 0x5: put("5", 1); break;
        case 0x6: put("6", 1); break; case 0x7: put("7", 1); break;
        case 0x8: put("8", 1); break; case 0x9: put("9", 1); break;
        case 0xa: put("a", 1); break; case 0xb: put("b", 1); break;
        case 0xc: put("c", 1); break; case 0xd: put("d", 1); break;
        case 0xe: put("e", 1); break; case 0xf: put("f", 1); break;
      }
      switch (bottom) {
        break; case 0x1: put("1", 1); break;
        case 0x2: put("2", 1); break; case 0x3: put("3", 1); break;
        case 0x4: put("4", 1); break; case 0x5: put("5", 1); break;
        case 0x6: put("6", 1); break; case 0x7: put("7", 1); break;
        case 0x8: put("8", 1); break; case 0x9: put("9", 1); break;
        case 0xa: put("a", 1); break; case 0xb: put("b", 1); break;
        case 0xc: put("c", 1); break; case 0xd: put("d", 1); break;
        case 0xe: put("e", 1); break; case 0xf: put("f", 1); break;
      }
      in.width -= sizeof(uint8_t);
    }
    put("}", 2);
  });

// clang-format on
#pragma clang diagnostic pop

void print_f_helper(struct print_arg p, um_fp typeName, outputFunction put,
                    um_fp args);

void print_f(outputFunction put, um_fp fmt, ...);

#ifndef __cplusplus
#define MAKE_PRINT_ARG_TYPE(type)                                              \
  type:                                                                        \
  um_from(#type)
#define MAKE_PRINT_ARG(a)                                                      \
  ((struct print_arg){.ref = REF(typeof(a), a),                                \
                      .name = _Generic((a),                                    \
                          MAKE_PRINT_ARG_TYPE(int),                            \
                          MAKE_PRINT_ARG_TYPE(um_fp),                          \
                          MAKE_PRINT_ARG_TYPE(char),                           \
                          MAKE_PRINT_ARG_TYPE(size_t),                         \
                          default: nullUmf)})
#else
#define MAKE_PRINT_ARG(a)                                                      \
  ((struct print_arg){                                                         \
      .ref = REF(typeof(a), a),                                                \
      .name = nullUmf,                                                         \
  })
#endif

#define EMPTY_PRINT_ARG ((struct print_arg){.ref = NULL, .name = nullUmf})

#define print_wf(printerfunction, fmt, ...)                                    \
  print_f(printerfunction,                                                     \
          um_from(fmt) __VA_OPT__(, APPLY_N(MAKE_PRINT_ARG, __VA_ARGS__)),     \
          EMPTY_PRINT_ARG)

#define println_wf(printerfunction, fmt, ...)                                  \
  print_wf(printerfunction, fmt "\n", __VA_ARGS__)
#define print(fmt, ...) print_wf(defaultPrinter, fmt, __VA_ARGS__)
#define println(fmt, ...) print_wf(defaultPrinter, fmt "\n", __VA_ARGS__)

#ifdef PRINTER_LIST_TYPENAMES
__attribute__((constructor(203))) static void post_init() {
  _um_fp_printer(defaultPrinter, REF(um_fp, um_from("list of type names:\n")));
  for (int i = 0; i < stringList_length(typeNamesList); i++) {
    _um_fp_printer(defaultPrinter, REF(um_fp, um_from(" ")));
    _um_fp_printer(defaultPrinter,
                   REF(um_fp, stringList_get(typeNamesList, i)));
    _um_fp_printer(defaultPrinter, REF(um_fp, um_from("\n")));
  }
}
#endif
__attribute__((hot)) static void defaultPrinter(char *c, unsigned int length) {
  fwrite(c, sizeof(char), length, stdout);
}

#endif

#ifdef PRINTER_C
inline unsigned int printer_arg_indexOf(um_fp string, char c) {
  int i;
  char *ptr = (char *)string.ptr;
  for (i = 0; i < string.width && ptr[i] != c; i++)
    ;
  return i;
}

inline um_fp printer_arg_until(char delim, um_fp string) {
  size_t i = 0;
  uint8_t *ptr = (uint8_t *)string.ptr;
  while (i < string.width && ptr[i] != delim)
    i++;
  string.width = i;
  return string;
}

inline um_fp printer_arg_after(char delim, um_fp slice) {
  size_t i = 0;
  uint8_t *ptr = slice.ptr;
  while (i < slice.width && ptr[i] != delim)
    i++;
  i = (i < slice.width) ? (i + 1) : (i);
  slice.ptr += i;
  slice.width -= i;
  return slice;
}
inline um_fp printer_arg_trim(um_fp in) {
  um_fp res = in;
  int front = 0;
  int back = in.width - 1;
  while (front < in.width && ((uint8_t *)in.ptr)[front] == ' ') {
    front++;
  }
  while (back > front && ((uint8_t *)in.ptr)[front] == ' ') {
    back--;
  }
  res = (um_fp){
      .ptr = in.ptr + front,
      .width = (size_t)(back - front + 1),
  };
  return res;
}

void print_f_helper(struct print_arg p, um_fp typeName, outputFunction put,
                    um_fp args) {
  void *ref = p.ref;
  if (!typeName.width) {
    typeName = p.name;
  }
  unsigned int index = stringList_search(typeNamesList, typeName);
  if (index >= printerFunctions->length) {
    USETYPEPRINTER(um_fp, um_from("_NO_TYPE("));
    USETYPEPRINTER(um_fp, typeName);
    USETYPEPRINTER(um_fp, um_from(")"));
  } else {
    (*((printerFunction *)List_getRef(printerFunctions, index)))(put, ref,
                                                                 args);
  }
}
#define um_charArr(um) ((char *)(um.ptr))
void print_f(outputFunction put, um_fp fmt, ...) {
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

        um_fp tname = printer_arg_until(':', typeName);
        um_fp parseargs = printer_arg_after(':', typeName);
        tname = printer_arg_trim(tname);
        print_f_helper(assumedName, tname, put, parseargs);
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
#undef um_charArr
#endif
