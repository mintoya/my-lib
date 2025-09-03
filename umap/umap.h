#ifndef UMAP_H
#define UMAP_H

#include "../my-list/my-list.h"
#include "../string-List/stringList.h"
#include <stdlib.h>
#include <string.h>

typedef struct umap {
  stringList* keys;
  stringList* vals;
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

#endif
