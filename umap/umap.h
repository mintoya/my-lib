#ifndef UMAP_H
#define UMAP_H

#include "../my-list/my-list.h"
#include "../printer/print.h"
#include "../string-List/stringList.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
  stringList *keys;
  stringList *vals;
  List *metadata;
} UMap;
typedef struct {
  um_fp raw;
} UMapView;
typedef enum {
  NORMAL = 0, // raw data
  LIST,       // umap with integer indexes
  MAP,        // umap with misc indexes
} UMap_innertype;

REGISTER_SPECIAL_PRINTER("UMap*", UMap *, {
  USETYPEPRINTER(um_fp, um_from("UMap_unimplemented"));
});
static inline um_fp UMap_getKeyAtIndex(UMap *map, unsigned int index) {
  return stringList_get(map->keys, index);
};
static inline um_fp UMap_getValAtIndex(UMap *map, unsigned int index) {
  return stringList_get(map->vals, index);
};
// clang-format off
static inline UMap_innertype 
UMap_getInnerType(UMap *map, unsigned int index) {
  UMap_innertype res = 
  ((index<(map->metadata->length))?(
    *(UMap_innertype *)List_getRef(map->metadata, index)
    ):(
      NORMAL
    ));
  return res;
};
// clang-format on
unsigned int UMap_binarySearch(UMap *map, um_fp key);
unsigned int UMapView_binarySearch(UMapView map, um_fp key);
UMap *UMap_new();
um_fp UMap_getValAtKey(UMap *map, um_fp key);
unsigned int UMap_set(UMap *map, um_fp key, um_fp val);
// DONT USE WITH AN ACTUAL UMAP
unsigned int UMap_setIndex(UMap *map, int index, um_fp val);
unsigned int UMap_setChildIndex(UMap *map, int i, UMap_innertype type,
                                UMap *ref);
// make a copy without unused keys & vals
UMap *UMap_remake(UMap *map);
void UMap_free(UMap *map);
unsigned int UMap_setChild(UMap *map, um_fp key, UMap_innertype, UMap *ref);

/*
 * not modifiable
 * freeing keyReg itself deletes this
 * free(*((void**)&result)) ?
 */
UMapView UMap_toBuf(UMap *map);
UMap UMap_fromBuf(UMapView mapRasterized);

um_fp UMapView_getKeyAtIndex(UMapView map, unsigned int index);
um_fp UMapView_getValAtIndex(UMapView map, unsigned int index);

stringListView UMapView_getVals(UMapView map);
stringListView UMapView_getKeys(UMapView map);
List UMapView_getMeta(UMapView map);
um_fp UMapView_getKeyAtIndex(UMapView map, unsigned int index);
um_fp UMapView_getValAtIndex(UMapView map, unsigned int index);
um_fp UMapView_getValAtKey(UMapView map, um_fp key);
void UMapView_free(UMapView umv);

#endif // UMAP_H
       //
