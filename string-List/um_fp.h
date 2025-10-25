#ifndef UM_FP_H
#define UM_FP_H
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    res = ((char *)a.ptr)[i] - ((char *)b.ptr)[i];
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

static inline void um_writeFile(char *filename, um_fp data) {
  FILE *f = fopen(filename, "w");
  if (!f) {
    fprintf(stderr, "couldnt write to file! :%s\n", filename);
    return;
  }
  fwrite(&data.width, sizeof(data.width), 1, f);
  fwrite(data.ptr, sizeof(char), data.width, f);
  fclose(f);
}
// on the stack
static um_fp um_readFile(char *filename) {
  um_fp res;
  FILE *f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "couldnt read from file! :%s\n", filename);
    return nullUmf;
  }
  if (fread(&(res.width), sizeof(res.width), 1, f) != 1) {
    fprintf(stderr, "file too small or corrupted %s\n", filename);
    fclose(f);
    return nullUmf;
  }
  res.ptr = (uint8_t *)malloc(res.width);
  if (fread(res.ptr, sizeof(char), res.width, f) != res.width) {
    fprintf(stderr, "file too small or corrupted %s\n", filename);
    fclose(f);
    return nullUmf;
  }
  fclose(f);
  return res;
}

#ifndef __cplusplus
#define um_fromT(type, val)                                                    \
  ((um_fp){.ptr = (type[1]){val}, .width = sizeof(type)})
#define um_fromP(ref, size) ((um_fp){.ptr = ref, .width = size})
#define um_from(val) ((um_fp){.ptr = ((uint8_t *)val), .width = strlen(val)})
#else
#include <string>
template <typename T> inline um_fp um_from(T &val) {
  return {(uint8_t *)&val, sizeof(T)};
}
template <> inline um_fp um_from<std::string>(std::string &s) {
  return {(uint8_t *)s.data(), s.size()};
}
template <> inline um_fp um_from<const std::string>(const std::string &s) {
  return {(uint8_t *)s.data(), s.size()};
}
inline um_fp um_from(const char *s) { return {(uint8_t *)s, strlen(s)}; }
template <size_t N> inline um_fp um_from(const char (&s)[N]) {
  return {(uint8_t *)s, N - 1};
}
#endif

#endif // UM_FP_H
