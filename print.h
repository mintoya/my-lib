#ifndef PRINTER_H
#define PRINTER_H
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

#include "allocator.h"
#include "hmap.h"
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

typedef void (*outputFunction)(const char *, unsigned int length, char flush);
typedef void (*printerFunction)(outputFunction, const void *, um_fp args);

#ifndef OVERRIDE_DEFAULT_PRINTER
#include <stdio.h>
static void stdoutPrint(const char *c, unsigned int length, char flush) {
  static struct {
    char buf[2048];
    size_t place;
  } buf = {
      .place = 0,
  };

  if (buf.place + length >= 2048 || flush) {
    fwrite(buf.buf, sizeof(char), buf.place, stdout);
    fwrite(c, sizeof(char), length, stdout);
    buf.place = 0;
  } else {
    memcpy(buf.buf + buf.place, c, length);
    buf.place += length;
  }
}
static outputFunction defaultPrinter = stdoutPrint;
#else
extern outputFunction defaultPrinter;
#endif

static
#ifdef __cplusplus
    thread_local
#else
    _Thread_local
#endif
    struct {
  HMap *data;
} PrinterSingleton;
static void PrinterSingleton_init() {
  PrinterSingleton.data = HMap_new(&defaultAllocator, 85);
}
static void PrinterSingleton_deinit() { HMap_free(PrinterSingleton.data); }
static void PrinterSingleton_append(um_fp name, printerFunction function) {
  HMap_set(PrinterSingleton.data, name,
           (um_fp){.ptr = (uint8_t *)(void *)REF(printerFunction, function),
                   .width = sizeof(printerFunction)});
}
static printerFunction lastprinters[2] = {NULL, NULL};
static um_fp lastnames[2] = {nullUmf, nullUmf};
static char lasttick = 0;
static printerFunction PrinterSingleton_get(um_fp name) {
  if (!um_fp_cmp(name, lastnames[lasttick])) {
    return lastprinters[lasttick];
  } else if (!um_fp_cmp(name, lastnames[!lasttick])) {
    return lastprinters[!lasttick];
  }
  struct HMap_both both = HMap_getBoth(PrinterSingleton.data, name);
  printerFunction p = NULL;
  if (both.val.width) {
    p = *(printerFunction *)(both.val.ptr);
    lasttick = !lasttick;
    lastprinters[lasttick] = p;
    lastnames[lasttick] = both.key;
  }
  return p;
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

#define PUTS(characters, length) put(characters, length, 0)
#define REGISTER_PRINTER(T, ...)                                               \
  __attribute__((weak)) void GETTYPEPRINTERFN(T)(                              \
      outputFunction put, const void *_v_in_ptr, um_fp args) {                 \
    (void)args;                                                                \
    T in = *(T *)_v_in_ptr;                                                    \
    __VA_ARGS__                                                                \
  }                                                                            \
  __attribute__((constructor(201))) static void register_##T() {               \
    um_fp key = (um_fp){.ptr = (uint8_t *)#T, .width = sizeof(#T) - 1};        \
    PrinterSingleton_append(key, GETTYPEPRINTERFN(T));                         \
  }

#define REGISTER_SPECIAL_PRINTER_NEEDID(id, str, type, ...)                    \
  __attribute__((weak)) void id(outputFunction put, const void *_v_in_ptr,     \
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
// the behavior of PUTS is modular
// 
// for building printers
// USENAMEDPRINTER(printerid,value)
// USETYPEPRINTER(type,value)
//
// typeprinter skips the search
// named printer skips the string parsing
// use print_wf with "put" as the ouputFunction 
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

  REGISTER_PRINTER(um_fp, {
    if (in.ptr) { PUTS((char *)in.ptr, in.width); } else { PUTS("__NULLUMF__", 11); }
  });
  typedef void *void_ptr;
  REGISTER_PRINTER(void_ptr, {
    uintptr_t v = (uintptr_t)in;
    PUTS("0x", 2);

    int shift = (sizeof(uintptr_t) * 8) - 4;
    int leading = 1;
    while (shift >= 0) {
      unsigned char nibble = (v >> shift) & 0xF;
      if (nibble || !leading || shift == 0) {
        leading = 0;
        char c = (nibble < 10) ? ('0' + nibble) : ('a' + nibble - 10);
        PUTS(&c, 1);
      }
      shift -= 4;
    }
  });
  typedef char *char_ptr;
  REGISTER_PRINTER(char_ptr, { while(*in){ PUTS(in,1); in++; } });

  REGISTER_PRINTER(float, {
    if (in < 0) {
      PUTS("-", 1);
      in *= -1;
    }
    int push = 0;
    while (((int)in)) {
      in /= 10;
      push++;
    }
    in *= 10;
    for (int i = 0; i < 6; i++) {
      char dig = '0';
      dig += ((unsigned int)in) % 10;
      PUTS(&dig, 1);
      if (i + 1 == push)
        PUTS(".", 1);
      in *= 10;
    }
  });
  REGISTER_PRINTER(double, {
    if (in < 0) {
      PUTS("-", 1);
      in *= -1;
    }
    int push = 0;
    while (((int)in)) {
      in /= 10;
      push++;
    }
    in *= 10;
    for (int i = 0; i < 12; i++) {
      char dig = '0';
      dig += ((unsigned int)in) % 10;
      PUTS(&dig, 1);
      if (i + 1 == push)
        PUTS(".", 1);
      in *= 10;
    }
  });
  REGISTER_PRINTER(int, {
    if (in < 0) {
      PUTS("-", 1);
      in = -in;
    }
    int l = 1;
    while (l <= in / 10)
      l *= 10;
    while (l) {
      char c = in / l + '0';
      PUTS(&c, 1);
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
      PUTS(&c, 1);
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


    PUTS("um_fp:", 6);
    if(useLength){
      USETYPEPRINTER(size_t, in.width);
    }else{
    }
    PUTS("{", 1);
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
          PUTS("(", 1);
          USETYPEPRINTER(int, counter);
          PUTS(")", 1);
        } else {
          while(counter){
            counter--;
            PUTS("0", 1);
          }
        }
        counter = 0;
      }

      // clang-format off
      switch (top) {
       case 0x1: PUTS("1", 1); break;
        case 0x2: PUTS("2", 1); break; case 0x3: PUTS("3", 1); break;
        case 0x4: PUTS("4", 1); break; case 0x5: PUTS("5", 1); break;
        case 0x6: PUTS("6", 1); break; case 0x7: PUTS("7", 1); break;
        case 0x8: PUTS("8", 1); break; case 0x9: PUTS("9", 1); break;
        case 0xa: PUTS("a", 1); break; case 0xb: PUTS("b", 1); break;
        case 0xc: PUTS("c", 1); break; case 0xd: PUTS("d", 1); break;
        case 0xe: PUTS("e", 1); break; case 0xf: PUTS("f", 1); break;
      }
      switch (bottom) {
        break; case 0x1: PUTS("1", 1); break;
        case 0x2: PUTS("2", 1); break; case 0x3: PUTS("3", 1); break;
        case 0x4: PUTS("4", 1); break; case 0x5: PUTS("5", 1); break;
        case 0x6: PUTS("6", 1); break; case 0x7: PUTS("7", 1); break;
        case 0x8: PUTS("8", 1); break; case 0x9: PUTS("9", 1); break;
        case 0xa: PUTS("a", 1); break; case 0xb: PUTS("b", 1); break;
        case 0xc: PUTS("c", 1); break; case 0xd: PUTS("d", 1); break;
        case 0xe: PUTS("e", 1); break; case 0xf: PUTS("f", 1); break;
      }
      in.width -= sizeof(uint8_t);
    }
    PUTS("}", 2);
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
MAKE_PRINT_ARG_TYPE(size_t);
#include "printer/genericName.h"
MAKE_PRINT_ARG_TYPE(void_ptr);
#include "printer/genericName.h"
MAKE_PRINT_ARG_TYPE(char_ptr);
#include "printer/genericName.h"
MAKE_PRINT_ARG_TYPE(float);
#include "printer/genericName.h"
MAKE_PRINT_ARG_TYPE(double);

// clang-format off
#define MAKE_PRINT_ARG(a)                                                      \
  ((struct print_arg){.ref = REF(typeof(a), a),                                \
                      .name = ((um_fp){.ptr=(uint8_t*)GENERIC_NAME(a),.width=sizeof(GENERIC_NAME(a))-1} )})

// clang-format on
#else
template <typename T> constexpr const char *type_name_cstr() { return ""; }

#define MAKE_PRINT_ARG_TYPE(type)                                              \
  template <> constexpr const char *type_name_cstr<type>() { return #type; }

MAKE_PRINT_ARG_TYPE(int);
MAKE_PRINT_ARG_TYPE(um_fp);
MAKE_PRINT_ARG_TYPE(char);
MAKE_PRINT_ARG_TYPE(size_t);
MAKE_PRINT_ARG_TYPE(void_ptr);
MAKE_PRINT_ARG_TYPE(char_ptr);
MAKE_PRINT_ARG_TYPE(float);
MAKE_PRINT_ARG_TYPE(double);

#define MAKE_PRINT_ARG(a)                                                      \
  ((struct print_arg){                                                         \
      .ref = REF(typeof(a), a),                                                \
      .name = um_from(type_name_cstr<std::decay_t<decltype(a)>>())})
#endif
#define EMPTY_PRINT_ARG ((struct print_arg){.ref = NULL, .name = nullUmf})

void print_f_helper(struct print_arg p, um_fp typeName, outputFunction put,
                    um_fp args);

void print_f(outputFunction put, um_fp fmt, ...);

#define print_wf(print, fmt, ...)                                              \
  print_f(print,                                                               \
          um_from("" fmt) __VA_OPT__(, APPLY_N(MAKE_PRINT_ARG, __VA_ARGS__)),  \
          EMPTY_PRINT_ARG)
#define println_wf(printerfunction, fmt, ...)                                  \
  print_wf(printerfunction, fmt "\n", __VA_ARGS__)
#define print(fmt, ...) print_wf(defaultPrinter, fmt, __VA_ARGS__)
#define println(fmt, ...) print_wf(defaultPrinter, fmt "\n", __VA_ARGS__)

#ifdef PRINTER_LIST_TYPENAMES
__attribute__((constructor(205))) static void post_init() {
  outputFunction put = defaultPrinter;
  println("==============================");
  println("printer debug");
  println("==============================");
  HMap_innertype *metas = PrinterSingleton.data->metadata;
  for (int i = 0; i < PrinterSingleton.data->metaSize; i++) {
    um_fp key = stringList_get(PrinterSingleton.data->KVs, metas[i].index);
    if (metas[i].hasindex) {
      println("${}", key);
      if (metas[i].hasnext) {
        HMap_innertype *h = metas + i;
        h = (HMap_innertype *)List_getRef(PrinterSingleton.data->links,
                                          h->next);

        while (h->hasindex) {
          um_fp key =
              stringList_get(PrinterSingleton.data->KVs, metas[i].index);
          println("   ${}",
                  stringList_get(PrinterSingleton.data->KVs, h->index));

          if (!h->hasnext)
            break;

          h = (HMap_innertype *)List_getRef(PrinterSingleton.data->links,
                                            h->next);
          if (!h)
            break;
        }
      }
    }
  }
  println("list of printer type names: ");
  println("footprint:  ${}", (size_t)HMap_footprint(PrinterSingleton.data));
  println("collisions: ${}", (int)HMap_countCollisions(PrinterSingleton.data));
  println("==============================");
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

void print_f_helper(struct print_arg p, um_fp typeName, outputFunction put,
                    um_fp args) {
  void *ref = p.ref;
  if (!typeName.width) {
    typeName = p.name;
  }
  printerFunction fn = PrinterSingleton_get(typeName);
  if (!fn) {
    USETYPEPRINTER(um_fp, um_from("__ NO_TYPE("));
    USETYPEPRINTER(um_fp, typeName);
    USETYPEPRINTER(um_fp, um_from(") __"));
  } else {
    fn(put, ref, args);
  }
}

#define um_charArr(um) ((char *)(um.ptr))
void print_f(outputFunction put, const um_fp fmt, ...) {
  va_list l;
  va_start(l, fmt);
  char check = 0;
  for (unsigned int i = 0; i < fmt.width; i++) {
    switch (um_charArr(fmt)[i]) {
    case '$':
      if (check)
        put(um_charArr(fmt) + i - 1, 1, 0);
      if (i + 1 == fmt.width)
        put("$", 1, 0);
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
          va_end(l);
          return put("__ NO ARGUMENT PROVIDED, ENDING PRINT __\n", 41, 1);
        }

        um_fp tname = printer_arg_until(':', typeName);
        um_fp parseargs = printer_arg_after(':', typeName);
        tname = printer_arg_trim(tname);
        print_f_helper(assumedName, tname, put, parseargs);
        i = j;
        check = 0;
      } else {
        put("{", 1, 0);
      }
      break;
    default:
      if (check)
        put(um_charArr(fmt) + i - 1, 1, 0);
      size_t ne;
      for (ne = 0; ne + i < fmt.width && um_charArr(fmt)[ne + i] != '$'; ne++)
        ;
      put(um_charArr(fmt) + i, ne, 0);
      i += (ne - 1);
      check = 0;
    }
  }
  va_end(l);
  put(NULL, 0, 1);
}
#undef um_charArr
#endif
