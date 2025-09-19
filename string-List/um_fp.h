#ifndef UM_FP_H
#define UM_FP_H
typedef struct fat_pointer {
  void *ptr;
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

static inline char um_eq(um_fp a, um_fp b) {
  return a.width == b.width && !um_fp_cmp(a, b);
}

#define nullUmf ((um_fp){.ptr = NULL, .width = 0})
#ifndef __cplusplus
#define um_fromT(type, val)                                             \
  ((um_fp){.ptr = (type[1]){val}, .width = sizeof(type)})
#define um_from(val)                                                           \
  _Generic((val),                                                              \
      char *: (um_fp){.ptr = (val), .width = strlen(val)},                     \
      default: (um_fp){.ptr = (typeof(val)[1]){val},                           \
                       .width = sizeof(typeof(val))}) // structs
#else
template <typename T>
inline um_fp um_from(T& val) {
    return { (void*)&val, sizeof(T) };
}

#endif

#endif // UM_FP_H
