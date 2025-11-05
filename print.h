#ifndef PRINTER_H
#define PRINTER_H
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

#include "printer/macros.h"
#include "printer/variadic.h"
#include "um_fp.h"

// temporary variables for compound literals
#ifdef __cplusplus
// clang-format off
  #include <type_traits>
  #include <utility>
  template <typename T> class StackPush {
  private:
    using RealT = std::remove_reference_t<T>;
    RealT value; 
  public:
    template <typename ArgType>
    explicit StackPush(ArgType &&arg) : value(std::forward<ArgType>(arg)) {}
    operator RealT &() { return value; }
    operator const RealT &() const { return value; }
    operator void *() { return (void *)&value; }
    operator const void *() const { return (const void *)&value; }
  };
// clang-format on
// #define typeof(m) decltype(m)
#define typeof(x) std::decay_t<decltype(x)>
#define REF(type, value) (StackPush<type>(value))
#else
#define REF(type, value) ((type[1]){value})
#endif

typedef void (*outputFunction)(char *, unsigned int length);
typedef void (*printerFunction)(outputFunction, void *, um_fp args);

typedef struct {
  um_fp n;
  size_t index;
  printerFunction fn;
} PrinterTuple;
static struct {
  PrinterTuple *elements;
  uint8_t *namesBuffer;
  size_t nbN;
  size_t N;
} PrinterSingleton;
static void PrinterSingleton_deinit() {
  free(PrinterSingleton.namesBuffer);
  free(PrinterSingleton.elements);
}
static void PrinterSingleton_append(um_fp name, printerFunction function) {
  PrinterSingleton.elements =
      (PrinterTuple *)realloc(PrinterSingleton.elements,
                              (PrinterSingleton.N + 1) * sizeof(PrinterTuple));
  PrinterSingleton.namesBuffer =
      (uint8_t *)realloc(PrinterSingleton.namesBuffer,
                         (PrinterSingleton.nbN + name.width) * sizeof(uint8_t));

  if (!PrinterSingleton.elements) {
    exit(ENOMEM);
  }
  PrinterTuple el = (PrinterTuple){
      .n = (um_fp){.ptr = NULL, .width = name.width},
      .index = PrinterSingleton.nbN,
      .fn = function,
  };
  PrinterSingleton.elements[PrinterSingleton.N] = el;
  memcpy(PrinterSingleton.namesBuffer +
             (sizeof(uint8_t) * PrinterSingleton.nbN),
         name.ptr, name.width);

  PrinterSingleton.N++;
  PrinterSingleton.nbN += name.width;
}
// static printerFunction PrinterSingleton_getLinear(um_fp name) {
//   for (int i = 0; i < PrinterSingleton.N; i++) {
//     um_fp element = PrinterSingleton.elements[i].n;
//     if (!um_fp_cmp(element, name))
//       return PrinterSingleton.elements[i].fn;
//   }
//   return NULL;
// }
static printerFunction PrinterSingleton_get(um_fp name) {
  static int comparisons = 0;
  unsigned int left = 0, right = PrinterSingleton.N - 1;
  while (left <= right) {
    int mid = left + (right - left) / 2;
    um_fp element = PrinterSingleton.elements[mid].n;

    int cmp = um_fp_cmp(element, name); // returns 0 if equal
    comparisons++;

    if (cmp == 0)
      return PrinterSingleton.elements[mid].fn;
    else if (cmp < 0)
      left = mid + 1;
    else
      right = mid - 1;
  }
  return NULL;
}

static um_fp PrinterSingleton_getN(int index) {
  return PrinterSingleton.elements[index].n;
}

static int comp(const void *a, const void *b) {
  PrinterTuple pa = *(PrinterTuple *)a;
  PrinterTuple pb = *(PrinterTuple *)b;
  return um_fp_cmp(pa.n, pb.n);
}
__attribute__((constructor(204))) static void PrinterSingleton_Sort() {
  for (int i = 0; i < PrinterSingleton.N; i++) {
    um_fp element = PrinterSingleton.elements[i].n;
    element.ptr =
        PrinterSingleton.namesBuffer + PrinterSingleton.elements[i].index;
    PrinterSingleton.elements[i].n = element;
  }
  qsort(PrinterSingleton.elements, PrinterSingleton.N, sizeof(PrinterSingleton),
        comp);
}
static void PrinterSingleton_init() {
  PrinterSingleton.elements = (PrinterTuple *)malloc(10 * sizeof(PrinterTuple));
  PrinterSingleton.namesBuffer = (uint8_t *)malloc(1024);
  PrinterSingleton.nbN = 0;
  PrinterSingleton.N = 0;
  if (!PrinterSingleton.elements) {
    exit(ENOMEM);
  }
}
// arg utils
// clang-format off

  unsigned int printer_arg_indexOf(um_fp string, char c);
  um_fp printer_arg_until(char delim, um_fp string) ;
  um_fp printer_arg_after(char delim, um_fp slice) ;
  um_fp printer_arg_trim(um_fp in) ;

// clang-format on

__attribute__((constructor(200))) static void printerInit() {
  PrinterSingleton_init();
}
__attribute__((destructor)) static void printerDeinit() {
  PrinterSingleton_deinit();
}

#define REGISTER_PRINTER(T, ...)                                               \
  __attribute__((weak)) void GETTYPEPRINTERFN(T)(                              \
      outputFunction put, void *_v_in_ptr, um_fp args) {                       \
    (void)args;                                                                \
    T in = *(T *)_v_in_ptr;                                                    \
    __VA_ARGS__                                                                \
  }                                                                            \
  __attribute__((constructor(201))) static void register_##T() {               \
    um_fp key = (um_fp){.ptr = (uint8_t *)#T, .width = sizeof(#T) - 1};        \
    PrinterSingleton_append(key, GETTYPEPRINTERFN(T));                         \
  }

#define REGISTER_SPECIAL_PRINTER_NEEDID(id, str, type, ...)                    \
  __attribute__((weak)) void id(outputFunction put, void *_v_in_ptr,           \
                                um_fp args) {                                  \
    type in = *(type *)_v_in_ptr;                                              \
    __VA_ARGS__                                                                \
  }                                                                            \
  __attribute__((constructor(202))) static void UNIQUE_PRINTER_FN2() {         \
    um_fp key = (um_fp){.ptr = (uint8_t *)str, .width = strlen(str)};          \
    PrinterSingleton_append(key, id);                                          \
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

#define REGISTER_ALIASED_PRINTER(realtype, alias)                              \
  __attribute__((constructor(201))) static void register__##alias() {          \
    um_fp key = (um_fp){.ptr = (uint8_t *)EXPAND_AND_STRINGIFY(alias),         \
                        .width = sizeof(EXPAND_AND_STRINGIFY(alias)) - 1};     \
    uint8_t *refFn =                                                           \
        (uint8_t *)(void *)REF(printerFunction, GETTYPEPRINTERFN(realtype));   \
    PrinterSingleton_append(key, GETTYPEPRINTERFN(realtype));                  \
  }

struct print_arg {
  void *ref;
  um_fp name;
};
// clang-format off

// examples with builtin types
// the behavior of "put" is modular
// 
// for building printers
// USENAMEDPRINTER(printerid,value)
// USETYPEPRINTER(type,value)
//
// typeprinter skips the search
// named printer skips the string parsing
// use print_wf with put as the ouputFunction 
// to keep output consistant, but that makes it recursive
// 
// you can pass args with a printerid and a colon 
// ex: "um_fp<void>: c0 length"
//
// #include "printer/genericName.h"
// MAKE_PRINT_ARG_TYPE(size_t);
// extends types assumed with ${} in c 
// 
// MAKE_PRINT_ARG_TYPE(size_t);
// does the same in cpp

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
// type assumption

#ifndef __cplusplus

#include "printer/genericName.h"
#define MAKE_PRINT_ARG_TYPE(type) MAKE_NEW_TYPE(type)

MAKE_PRINT_ARG_TYPE(int);
#include "printer/genericName.h"
MAKE_PRINT_ARG_TYPE(um_fp);
#include "printer/genericName.h"
MAKE_PRINT_ARG_TYPE(char);
#include "printer/genericName.h"
MAKE_PRINT_ARG_TYPE(size_t);

// clang-format off
#define MAKE_PRINT_ARG(a)                                                      \
  ((struct print_arg){.ref = REF(typeof(a), a),                                \
                      .name = ((um_fp){.ptr=(uint8_t*)GENERIC_NAME(a),.width=sizeof(GENERIC_NAME(a))-1} )})

// clang-format on
#else
template <typename T> constexpr const char *type_name_cstr() {
  return "unknown";
}

#define MAKE_PRINT_ARG_TYPE(type)                                              \
  template <> constexpr const char *type_name_cstr<type>() { return #type; }

MAKE_PRINT_ARG_TYPE(int);
MAKE_PRINT_ARG_TYPE(um_fp);
MAKE_PRINT_ARG_TYPE(char);
MAKE_PRINT_ARG_TYPE(size_t);

#define MAKE_PRINT_ARG(a)                                                      \
  ((struct print_arg){                                                         \
      .ref = REF(typeof(a), a),                                                \
      .name = um_from(type_name_cstr<std::decay_t<decltype(a)>>())})
#endif
#define EMPTY_PRINT_ARG ((struct print_arg){.ref = NULL, .name = nullUmf})

void print_f_helper(struct print_arg p, um_fp typeName, outputFunction put,
                    um_fp args);

static struct {
  char buffer[1024];
  uint16_t place;
} print_f_singleton;
static void print_f_singleton_start(outputFunction put) {
  put(print_f_singleton.buffer, print_f_singleton.place);
  print_f_singleton.place = 0;
}
static void print_f_singleton_end(outputFunction put) {
  put(print_f_singleton.buffer, print_f_singleton.place);
  print_f_singleton.place = 0;
}

static void print_f_singleton_write(outputFunction put, char *ptr,
                                    unsigned int len) {
#if PRINTER_UNBUFFERED
  put(ptr, len);
#else
  if (print_f_singleton.place + len > sizeof(print_f_singleton.buffer)) {
    put(print_f_singleton.buffer, print_f_singleton.place);
    put(ptr, len);
    print_f_singleton.place = 0;
  } else {
    memcpy(print_f_singleton.buffer + print_f_singleton.place, ptr, len);
    print_f_singleton.place += len;
  }
#endif
}
void print_f(outputFunction put, um_fp fmt, ...);

#define print_wf(print, fmt, ...)                                              \
  print_f(print,                                                               \
          um_from("" fmt) __VA_OPT__(, APPLY_N(MAKE_PRINT_ARG, __VA_ARGS__)),  \
          EMPTY_PRINT_ARG)
#define println_wf(printerfunction, fmt, ...)                                  \
  print_wf(printerfunction, fmt "\n", __VA_ARGS__)
#define print(fmt, ...) print_wf(defaultPrinter, fmt, __VA_ARGS__)
#define println(fmt, ...) print_wf(defaultPrinter, fmt "\n", __VA_ARGS__)

#include <stdio.h>
__attribute__((weak)) void defaultPrinter(char *c, unsigned int length) {
  fwrite(c, sizeof(char), length, stdout);
}

#ifdef PRINTER_LIST_TYPENAMES
__attribute__((constructor(205))) static void post_init() {
  outputFunction put = defaultPrinter;
  USETYPEPRINTER(um_fp, um_from("list of printer type names:\n"));
  for (int i = 0; i < PrinterSingleton.N; i++) {
    USETYPEPRINTER(um_fp, um_from(" "));
    USETYPEPRINTER(um_fp, PrinterSingleton_getN(i));
    USETYPEPRINTER(um_fp, um_from("\n"));
  }
}
#endif // PRINTER_LIST_TYPENAMES

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

static outputFunction putstore = NULL;
void put_temp(char *buffer, unsigned int len) {
  if (putstore) {
    print_f_singleton_write(putstore, buffer, len);
  }
}
void print_f_helper(struct print_arg p, um_fp typeName, outputFunction put,
                    um_fp args) {
  void *ref = p.ref;
  if (!typeName.width) {
    typeName = p.name;
  }
  printerFunction fn = PrinterSingleton_get(typeName);
  if (!fn) {
    USETYPEPRINTER(um_fp, um_from("_NO_TYPE("));
    USETYPEPRINTER(um_fp, typeName);
    USETYPEPRINTER(um_fp, um_from(")"));
  } else {
    putstore = put;
    fn(put_temp, ref, args);
    putstore = NULL;
  }
}

#define um_charArr(um) ((char *)(um.ptr))
void print_f(outputFunction put, um_fp fmt, ...) {
  print_f_singleton_start(put);
  va_list l;
  va_start(l, fmt);
  char check = 0;
  for (unsigned int i = 0; i < fmt.width; i++) {
    switch (um_charArr(fmt)[i]) {
    case '$':
      if (check)
        print_f_singleton_write(put, um_charArr(fmt) + i - 1, 1);
      if (i + 1 == fmt.width)
        print_f_singleton_write(put, "$", 1);
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
        if (!assumedName.ref) {
          print_f_singleton_end(put);
          return put("__ NO ARGUMENT PROVIDED, ENDING PRINT __", 40);
        }

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
        print_f_singleton_write(put, um_charArr(fmt) + i - 1, 1);
      print_f_singleton_write(put, um_charArr(fmt) + i, 1);
      check = 0;
    }
  }
  va_end(l);
  print_f_singleton_end(put);
}
#undef um_charArr
#endif
