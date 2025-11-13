#ifndef UM_FP_H
#define UM_FP_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct fat_pointer {
  uint8_t *ptr;
  size_t width;
} um_fp;

// #include <string.h>
static inline int um_fp_cmp(const um_fp a, const um_fp b) {
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
  if (!res)
    return 0;
  return (res < 0 ? -1 : 1);
}

static char um_eq(um_fp a, um_fp b) { return !um_fp_cmp(a, b); }

#define UM_DEFAULT(...) {__VA_ARGS__}
#define UM_CASE(fp, ...)                                                       \
  if (um_eq(um_from(fp), __temp)) {                                            \
    __VA_ARGS__                                                                \
  } else
#define UM_SWITCH(fp, ...)                                                     \
  do {                                                                         \
    um_fp __temp = fp;                                                         \
    __VA_ARGS__                                                                \
  } while (0)

#ifdef __cplusplus
inline bool operator==(const um_fp &a, const um_fp &b) { return um_eq(a, b); }
inline bool operator!=(const um_fp &a, const um_fp &b) { return !um_eq(a, b); }
#endif

#define nullUmf ((um_fp){.ptr = (uint8_t *)NULL, .width = 0})

#define um_block(var)                                                          \
  ((um_fp){.ptr = (uint8_t *)(typeof(var)[1]){var},                            \
           .width = sizeof(typeof(var))})
#define um_blockT(type, ...)                                                   \
  ((um_fp){.ptr = (uint8_t *)(type[1]){__VA_ARGS__}, .width = sizeof(type)})

#ifndef __cplusplus
#define um_fromP(ref, size) ((um_fp){.ptr = ref, .width = size})
#define um_from(val)                                                           \
  _Generic((val),                                                              \
      um_fp: val,                                                              \
      default: ((um_fp){.ptr = ((uint8_t *)val), .width = strlen(val)}))
#else
#include <cstdint>
#include <cstring>
#include <string>
template <typename T> inline um_fp um_from(T &val) {
  return {reinterpret_cast<uint8_t *>(&val), sizeof(T)};
}
// inline um_fp um_from(std::string &s) {
//   return {reinterpret_cast<uint8_t *>(s.data()), s.size()};
// }
inline um_fp um_from(um_fp u) { return u; }
inline um_fp um_from(const std::string &s) {
  return {const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(s.data())),
          s.size()};
}
inline um_fp um_from(const char *s) {
  return {const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(s)),
          std::strlen(s)};
}
template <size_t N> inline um_fp um_from(const char (&s)[N]) {
  return {const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(s)), N - 1};
}

#endif

#endif // UM_FP_H
