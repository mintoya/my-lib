
#define GETTYPEPRINTERFN(T) _##T##_printer
#define MERGE_PRINTER_M(a, b) a##b
#define LABEL_PRINTER_GEN(l, a) MERGE_PRINTER_M(l, a)

#define UNIQUE_GEN_PRINTER                                                     \
  LABEL_PRINTER_GEN(LABEL_PRINTER_GEN(__LINE__, _), __COUNTER__)
#define UNIQUE_PRINTER_FN LABEL_PRINTER_GEN(PRINTERFN, UNIQUE_GEN_PRINTER)
#define UNIQUE_PRINTER_FN2                                                     \
  LABEL_PRINTER_GEN(printerConstructor, UNIQUE_GEN_PRINTER)

