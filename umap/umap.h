#ifndef UMAP_H
#define UMAP_H

#include "../my-list/my-list.h"
#include <stdlib.h>
#include <string.h>

typedef struct fat_pointer {
  void *ptr;
  unsigned int length;
} um_fp;
typedef struct umap {
  List *keyReg; // umapListElements
  List *keys;   // chars
  List *valReg; // umapListElements
  List *vals;   // chars
} UMap;

UMap *UMap_new();
um_fp UMap_getValAtKey(UMap *map, um_fp key);
um_fp UMap_getKeyAtIndex(UMap *map, unsigned int index);
um_fp UMap_get(UMap *map, char *key);
void UMap_set(UMap *map, um_fp key, um_fp val);
// make a copy without unused keys & vals
UMap *UMap_remake(UMap *map);
void UMap_free(UMap *map);

/*
 * not modifiable
 * freeing keyReg itself deletes this
 * free(*((void**)&result)) ?
 */
UMap UMap_fromBuf(um_fp mapRasterized);
um_fp UMap_toBuf(UMap *map);

#define nullUmf ((um_fp){.ptr = NULL, .length = 0})
#define um_from(val)                                                           \
  _Generic((val),                                                              \
      char *: (um_fp){.ptr = (val), .length = strlen(val)},                    \
      const char *: (um_fp){.ptr = (val), .length = strlen(val)},              \
      default: (um_fp){.ptr = &(val), .length = sizeof(val)})


#endif
