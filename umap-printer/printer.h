#ifndef UPRINTER_H
#define UPRINTER_H

#include "../umap/umap.h"
typedef void (*printer)(const void *);
void registerPrinter(char *, printer *);
void mPrint(char *type, const void *ref);
#define addPrinter(type, fn) registerPrinter(#type, (printer[1]){fn})
#define usePrint(type, val) mPrint(#type, (type[1]){val})
#define usePrintln(type, val)                                                  \
  usePrint(type, val);                                                         \
  usePrint(char *, "\n");

#endif // !UPRINTER_H
