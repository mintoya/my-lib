
#define GETTYPEPRINTERFN(T) _##T##_printer
#define MERGE_PRINTER_M(a, b) a##b
#define LABEL_PRINTER_GEN(l, a) MERGE_PRINTER_M(l, a)

#define UNIQUE_GEN_PRINTER                                                     \
  LABEL_PRINTER_GEN(LABEL_PRINTER_GEN(__LINE__, _), __COUNTER__)
#define UNIQUE_PRINTER_FN LABEL_PRINTER_GEN(PRINTERFN, UNIQUE_GEN_PRINTER)
#define UNIQUE_PRINTER_FN2                                                     \
  LABEL_PRINTER_GEN(printerConstructor, UNIQUE_GEN_PRINTER)

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
