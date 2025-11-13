#ifndef UM_FP_H
#define UM_FP_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct fat_pointer {
  uint8_t *ptr;
  size_t width;
} fptr;

typedef struct {
  uint8_t *ptr;
  size_t width;
  size_t capacity;
} ffptr;

typedef fptr um_fp;

// #include <string.h>
static inline int fptr_cmp(const fptr a, const fptr b) {
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

static char fptr_eq(fptr a, fptr b) { return !um_fp_cmp(a, b); }
static char (*um_eq)(fptr, fptr) = fptr_eq;

#define UM_DEFAULT(...) {__VA_ARGS__}
#define UM_CASE(fp, ...)                                                       \
  if (um_eq(fp_from(fp), __temp)) {                                            \
    __VA_ARGS__                                                                \
  } else
#define UM_SWITCH(fp, ...)                                                     \
  do {                                                                         \
    fptr __temp = fp;                                                          \
    __VA_ARGS__                                                                \
  } while (0)

#ifdef __cplusplus
inline bool operator==(const fptr &a, const fptr &b) { return um_eq(a, b); }
inline bool operator!=(const fptr &a, const fptr &b) { return !um_eq(a, b); }
#endif

#define nullUmf ((fptr){.ptr = (uint8_t *)NULL, .width = 0})
#define nullFptr ((fptr){.ptr = (uint8_t *)NULL, .width = 0})
#define nullFFptr ((ffptr){.ptr = (uint8_t *)NULL, .width = 0, .capacity = 0})

#define um_block(var)                                                          \
  ((fptr){.ptr = (uint8_t *)(typeof(var)[1]){var},                             \
          .width = sizeof(typeof(var))})
#define um_blockT(type, ...)                                                   \
  ((fptr){.ptr = (uint8_t *)(type[1]){__VA_ARGS__}, .width = sizeof(type)})

#ifndef __cplusplus

#define is_comparr(x)                                                          \
  (!__builtin_types_compatible_p(__typeof__(x), __typeof__(&(x)[0])))
#define fp_fromP(ref, size) ((fptr){.ptr = ref, .width = size})
#define fp_from(val)                                                           \
  _Generic(&(val),                                                             \
      fptr *: *(fptr *)&(val),                                                 \
      ffptr *: ((fptr){                                                        \
                 .ptr = ((ffptr *)&(val))->ptr,                                \
                 .width = ((ffptr *)&(val))->width,                            \
             }),                                                               \
      default: ((fptr){                                                        \
          .ptr = (uint8_t *)(val),                                             \
          .width = (is_comparr(val) ? sizeof(val) - 1                          \
                                    : strlen((const char *)(val))),            \
      }))

#else
#include <cstdint>
#include <cstring>
#include <string>
template <typename T> inline fptr fp_from(T &val) {
  return {reinterpret_cast<uint8_t *>(&val), sizeof(T)};
}
// inline fptr fp_from(std::string &s) {
//   return {reinterpret_cast<uint8_t *>(s.data()), s.size()};
// }
inline fptr fp_from(fptr u) { return u; }
inline fptr fp_from(const std::string &s) {
  return {const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(s.data())),
          s.size()};
}
inline fptr fp_from(const char *s) {
  return {const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(s)),
          std::strlen(s)};
}
template <size_t N> inline fptr fp_from(const char (&s)[N]) {
  return {const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(s)), N - 1};
}

#endif

#endif // UM_FP_H
