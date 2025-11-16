#ifndef UM_FP_H
#define UM_FP_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  size_t width;
  uint8_t *ptr;
} fptr;

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
  fptr fptr;
} ffptr;

typedef fptr um_fp;

// #include <string.h>
[[gnu::pure]] static inline int fptr_cmp(const fptr a, const fptr b) {
  int wd = a.width - b.width;
  if (wd) {
    return wd;
  }
  if (!a.width) {
    return 0;
  }
  // return
  // memcmp(a.ptr, b.ptr, a.width);
  int res = 0;
  size_t top = a.width / sizeof(intmax_t);
  size_t bottom = a.width % sizeof(intmax_t);
  intmax_t *starta = (intmax_t *)a.ptr;
  intmax_t *startb = (intmax_t *)b.ptr;
  uint8_t *resta = a.ptr + (top * (sizeof(intmax_t)));
  uint8_t *restb = b.ptr + (top * (sizeof(intmax_t)));
  for (size_t i = 0; i < top && !res; i++)
    res = (starta[i] - startb[i]);
  for (size_t i = 0; i < bottom && !res; i++)
    res = (resta[i] - restb[i]);
  int result;
  if (!res)
    result = 0;
  else if (res < 0)
    result = -1;
  else
    result = 1;
  return result;
}
static int (*um_fp_cmp)(const fptr, const fptr) = fptr_cmp;

#include <string.h>
static inline void fpmemset(uint8_t *ptr, const fptr element, size_t ammount) {
  size_t set = 0;
  while (set < ammount) {
    if (!set) {
      memcpy(ptr, element.ptr, element.width);
      set++;
    } else {
      size_t toset = set < (ammount - set) ? set : ammount - set;
      memcpy(ptr + set * element.width, ptr, element.width * toset);
      set *= 2;
    }
  }
}

static char fptr_eq(fptr a, fptr b) { return !fptr_eq(a, b); }
static char (*um_eq)(fptr, fptr) = fptr_eq;

#ifdef __cplusplus
inline bool operator == (const fptr &a, const fptr &b) { return fptr_eq(a, b); }
inline bool operator != (const fptr &a, const fptr &b) { return !fptr_eq(a, b); }
#endif
#ifdef __cplusplus
#define typeof(x) std::decay_t<decltype(x)>
#endif
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
#define lmemset(arr, arrlen, ...)                       \
  do {                                                  \
    typeof(__VA_ARGS__) __temp = (__VA_ARGS__);         \
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
template<typename T> inline fptr fp_from(T &val) {
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
template<size_t N> inline fptr fp_from(const char (&s)[N]) {
  return {
      .width = N - 1,
      .ptr = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(s)),
  };
}

#endif

#endif // UM_FP_H
