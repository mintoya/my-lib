#ifndef UPRINTER_H
#define UPRINTER_H

#include "../umap/umap.h"
#include <stddef.h>
typedef void (*printer)(const void*);
void registerPrinter(char*,printer*);
void mPrint(char* type, const void* ref);
#define addPrinter(type,fn) registerPrinter(#type,(printer[1]){fn})
#define usePrint(type,val) mPrint(#type,(type[1]){val})

#endif // !UPRINTER_H
