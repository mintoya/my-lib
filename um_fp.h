#ifndef UM_FP_H
#define UM_FP_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct fat_pointer {
  uint8_t *ptr;
  size_t width;
} um_fp;

static inline int um_fp_cmp(um_fp a, um_fp b) {
  if (a.width != b.width) {
    return (a.width < b.width) ? -1 : 1;
  }
  if (a.width == 0) {
    return 0;
  }
  int res = 0;
  unsigned int i = 0;
  for (unsigned int i = 0; i < a.width && !res; i++) {
    res = ((uint8_t *)a.ptr)[i] - ((uint8_t *)b.ptr)[i];
  }
  return res;
}

static char um_eq(um_fp a, um_fp b) {
  return a.width == b.width && !um_fp_cmp(a, b);
}

#ifdef __cplusplus
inline bool operator==(const um_fp &a, const um_fp &b) { return um_eq(a, b); }
inline bool operator!=(const um_fp &a, const um_fp &b) { return !um_eq(a, b); }
#endif

#define nullUmf ((um_fp){.ptr = NULL, .width = 0})

#define um_block(var)                                                          \
  ((um_fp){.ptr = (uint8_t *)(typeof(var)[1]){var},                            \
           .width = sizeof(typeof(var))})
#define um_blockT(type, ...)                                                   \
  ((um_fp){.ptr = (uint8_t *)(type[1]){__VA_ARGS__}, .width = sizeof(type)})

#ifndef __cplusplus
#define um_fromP(ref, size) ((um_fp){.ptr = ref, .width = size})
#define um_from(val) ((um_fp){.ptr = ((uint8_t *)val), .width = strlen(val)})
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
