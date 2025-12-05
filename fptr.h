#ifndef UM_FP_H
#define UM_FP_H
#ifdef _WIN32
// #include <malloc.h>
// #else
// #include <alloca.h>
#endif
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
// #include <wchar.h>

typedef wchar_t wchar;
typedef unsigned int uint;
typedef unsigned char uchar;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uintmax_t umax;
typedef intmax_t imax;
typedef size_t usize;

#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)

#ifndef __cplusplus
#ifndef thread_local
#define thread_local _Thread_local
#endif
#endif

typedef struct {
  size_t width;
  u8 *ptr;
} fptr;
typedef struct {
  size_t width;
  u8 ptr[];
} bFptr;
#define fptr_fromB(bfptr) ((fptr){.width = (bfptr).width, .ptr = (uint8_t *)(bfptr).ptr})

struct fatter_pointer {
  fptr fpart;
  size_t capacity;
};

typedef union {
  struct
  {
    fptr fpart;
    size_t capacity;
  } ffptr;
  fptr fptrp;
} ffptr;

typedef fptr um_fp;

#include <string.h>
extern inline fptr fptr_fromCS(char *cstr) {
  return (fptr){(size_t)strlen(cstr), (u8 *)cstr};
}
extern inline fptr fptr_fromPL(u8 *cstr, size_t len) {
  return (fptr){len, (u8 *)cstr};
}
[[gnu::pure]] extern inline int fptr_cmp(const fptr a, const fptr b) {
  int wd = a.width - b.width;
  if (wd) {
    return wd;
  }
  if (!a.width) {
    return 0;
  }
#ifndef IGNORE_ALIGNMENT
  return memcmp(a.ptr, b.ptr, a.width);
#else
  int res = 0;
  size_t top = a.width / sizeof(intmax_t);
  size_t bottom = a.width % sizeof(intmax_t);
  u8 *resta = a.ptr + (top * (sizeof(imax)));
  u8 *restb = b.ptr + (top * (sizeof(imax)));
  for (size_t i = 0; i < top && !res; i++) {
    imax ai;
    imax bi;
    ai = *(imax *)(a.ptr + sizeof(imax) * i);
    bi = *(imax *)(b.ptr + sizeof(imax) * i);
    res = ai - bi;
  }
  for (size_t i = 0; i < bottom && !res; i++)
    res = (resta[i] - restb[i]);
  int result;
  if (!res)
    return 0;
  else if (res < 0)
    return -1;
  else
    return 1;
#endif
}
static int (*um_fp_cmp)(const fptr, const fptr) = fptr_cmp;

#include <string.h>
extern inline void fpmemset(uint8_t *ptr, const fptr element, size_t ammount) {
  if (!ammount)
    return;
  memcpy(ptr, element.ptr, element.width);
  for (size_t set = 1; set < ammount; set *= 2) {
    size_t toset = ammount - set;
    toset = set < toset ? set : toset;
    memcpy(ptr + set * element.width, ptr, element.width * toset);
  }
}

extern inline char fptr_eq(fptr a, fptr b) { return !fptr_cmp(a, b); }

#ifdef __cplusplus
inline bool operator==(const fptr &a, const fptr &b) { return fptr_eq(a, b); }
inline bool operator!=(const fptr &a, const fptr &b) { return !fptr_eq(a, b); }
#define typeof(x) std::decay_t<decltype(x)>
#endif

#define setvar_aligned(var, ptr)                   \
  do {                                             \
    const size_t alignment = alignof(typeof(var)); \
    const uintptr_t address = (uintptr_t)(ptr);    \
                                                   \
    if (!(address & (alignment - 1))) {            \
      var = *(typeof(var) *)(ptr);                 \
    } else {                                       \
      memcpy(&(var), (ptr), sizeof(typeof(var)));  \
    }                                              \
  } while (0)

#define align_alloca(type) ({                                           \
  uintptr_t newptr = (uintptr_t)alloca(sizeof(type) + alignof(type));   \
  newptr += (alignof(type) - (newptr % alignof(type))) % alignof(type); \
  (type *)newptr;                                                       \
})

#ifndef __cplusplus
#define REF(type, value) ((type[1]){value})
#else
// #include <type_traits>
// #include <utility>
// template <typename T>
// class StackPush {
// private:
//   using RealT = std::remove_reference_t<T>;
//   RealT value;
//
// public:
//   template <typename ArgType>
//   explicit StackPush(ArgType &&arg) : value(std::forward<ArgType>(arg)) {}
//   operator RealT &() { return value; }
//   operator const RealT &() const { return value; }
//   operator void *() { return (void *)&value; }
//   operator const void *() const { return (const void *)&value; }
// };
// #define REF(type, value) (StackPush<type>(value))
template <typename T>
static inline T *ref_tmp(T &&v) {
  return &v;
}
#define REF(type, value) ref_tmp(type{value})

#endif

#define fptr_stack_split(string, ...) ({fptr* __temp__result = (fptr*)alloca( (sizeof((unsigned int[]){__VA_ARGS__}) / sizeof(unsigned int) + 1)*sizeof(fptr) ); \
  do {                                                                                 \
    uint8_t *last;                                                                     \
    unsigned int args[] = {__VA_ARGS__};                                               \
    for (int i = 0; i < sizeof(args) / sizeof(unsigned int); i++) {                    \
      args[i] = (i == 0)                                                               \
                    ? ((args[i] < string.width)                                        \
                           ? args[i]                                                   \
                           : string.width)                                             \
                    : ((string.width < ((args[i] > args[i - 1])                        \
                                            ? args[i]                                  \
                                            : args[i - 1]))                            \
                           ? string.width                                              \
                           : ((args[i] > args[i - 1])                                  \
                                  ? args[i]                                            \
                                  : args[i - 1]));                                     \
      __temp__result[i] = (fptr){                                                              \
          .width = (i == 0) ? (args[0]) : (args[i] - args[i - 1]),                     \
          .ptr = (i == 0) ? (string.ptr) : (last),                                     \
      };                                                                               \
      last = ((uint8_t *)__temp__result[i].ptr) + __temp__result[i].width;                             \
    }                                                                                  \
    __temp__result[sizeof(args) / sizeof(unsigned int)] = (fptr){                              \
        .width = string.width - ((uint8_t *)last - (uint8_t *)string.ptr),             \
        .ptr = last,                                                                   \
    };                                                                                 \
  } while (0);__temp__result; })
#define isSkip(char) ( \
    char == ' ' ||     \
    char == '\n' ||    \
    char == '\r' ||    \
    char == '\t'       \
)
#define UM_DEFAULT(...) {__VA_ARGS__}
#define UM_CASE(fp, ...)     \
  if (fptr_eq(fp, __temp)) { \
    __VA_ARGS__              \
  } else
#define UM_SWITCH(fp, ...) \
  do {                     \
    fptr __temp = fp;      \
    __VA_ARGS__            \
  } while (0)
#define lmemset(arr, arrlen, value)                     \
  do {                                                  \
    typeof(value) __temp = (value);                     \
    fptr __tempFp = ((fptr){                            \
        .width = sizeof(__temp),                        \
        .ptr = ((uint8_t *)&(__temp)),                  \
    });                                                 \
    fpmemset(((uint8_t *)(arr)), (__tempFp), (arrlen)); \
  } while (0)
#define nullFptr ((fptr){0})
#define nullUmf nullFptr
#define nullFFptr ((ffptr){0})

#define um_block(var)                          \
  ((fptr){                                     \
      .width = sizeof(var),                    \
      .ptr = (uint8_t *)(typeof(var)[1]){var}, \
  })
#define um_blockT(type, ...)                    \
  ((fptr){                                      \
      .width = sizeof(type),                    \
      .ptr = (uint8_t *)(type[1]){__VA_ARGS__}, \
  })

#define structEq(a, b)              \
  (fptr_eq(                         \
      (fptr){                       \
          .width = sizeof(a),       \
          .ptr = (uint8_t *)(&(a)), \
      },                            \
      (fptr){                       \
          .width = sizeof(b),       \
          .ptr = (uint8_t *)(&(b)), \
      }                             \
  ))

#ifndef __cplusplus

#define fp_fromT(struct)           \
  ((fptr){                         \
      .ptr = (uint8_t *)&(struct), \
      .width = sizeof(struct),     \
  })
#define fp_fromP(ref, size)  \
  ((fptr){                   \
      .ptr = (uint8_t *)ref, \
      .width = size,         \
  })
#define is_comparr(x) \
  (!__builtin_types_compatible_p(__typeof__(x), __typeof__(&(x)[0])))
#define fp_from(val)                                                         \
  ((fptr){                                                                   \
      .width =                                                               \
          (is_comparr(val) ? sizeof(val) - 1 : strlen((const char *)(val))), \
      .ptr = (uint8_t *)(val),                                               \
  })

#else
#include <cstdint>
#include <cstring>
#include <string>
template <typename T>
inline fptr fp_from(T &val) {
  return {
      .width = sizeof(T),
      .ptr = reinterpret_cast<uint8_t *>(&val),
  };
}
inline fptr fp_from(fptr u) { return u; }
inline fptr fp_from(const std::string &s) {
  return {
      .width = s.size(),
      .ptr = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(s.data())),
  };
}
inline fptr fp_from(const char *s) {
  return {
      .width = std::strlen(s),
      .ptr = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(s)),
  };
}
template <size_t N>
inline fptr fp_from(const char (&s)[N]) {
  return {
      .width = N - 1,
      .ptr = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(s)),
  };
}

#endif
static fptr fptr_trim(fptr in) {
  while (isSkip(*in.ptr)) {
    in.ptr++;
    in.width--;
  }
  while (isSkip(*(in.ptr + in.width - 1))) {
    in.width--;
  }
  return in;
}
#undef isSkip
#define isDigit(char) (char <= '9' && char >= '0')
static unsigned int fptr_toUint(const fptr in) {
  fptr copy = fptr_trim(in);
  unsigned int res = 0;
  for (size_t place = 0; place < copy.width && isDigit(copy.ptr[place]); place++) {
    res *= 10;
    res += copy.ptr[place] - '0';
  }
  return res;
}
#undef isDigit
static int fptr_toInt(const fptr in) {
  fptr number = fptr_trim(in);
  if (!number.width)
    return 0;
  char negetive = number.ptr[0] == '-';
  if (negetive) {
    number.ptr++;
    number.width--;
  }
  return (negetive ? -1 : 1) * fptr_toUint(number);
}
#endif // UM_FP_H
