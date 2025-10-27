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
  stringList *vals;
  List *metadata;
} UMapList;
typedef struct {
  um_fp raw;
} UMapView;
typedef struct {
  um_fp raw;
} UMapListView;
typedef enum {
  NORMAL = 0, // raw data
  LIST,       // umap with integer indexes
  MAP,        // umap with misc indexes
} UMap_innertype;

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
UMapList *UMapList_new();

um_fp UMap_getValAtKey(UMap *map, um_fp key);
unsigned int UMap_set(UMap *map, um_fp key, um_fp val);
// DONT USE WITH AN ACTUAL UMAP
// make a copy without unused keys & vals
UMap *UMap_remake(UMap *map);
void UMap_free(UMap *map);
unsigned int UMap_setChild(UMap *map, um_fp key, UMap *ref);
unsigned int UMap_setList(UMap *map, um_fp key, UMapList *ref);
unsigned int UMapList_setChild(UMapList *map, unsigned int key, UMap *ref);
unsigned int UMapList_setList(UMapList *map, unsigned int key, UMapList *ref);

/*
 * not modifiable
 * freeing keyReg itself deletes this
 */
UMapView UMap_toBuf(UMap *map);
UMapListView UMapList_toBuf(UMapList *map);
UMap UMap_fromBuf(UMapView mapRasterized);
UMap UMapList_fromBuf(UMapListView mapRasterized);

um_fp UMapView_getKeyAtIndex(UMapView map, unsigned int index);
um_fp UMapView_getValAtIndex(UMapView map, unsigned int index);
unsigned int UMapList_set(UMapList *map, unsigned int index, um_fp val);
unsigned int UMapList_append(UMapList *map, um_fp val);
um_fp UMapList_get(UMapList *map, unsigned int key);

stringListView UMapView_getVals(UMapView map);
stringListView UMapView_getKeys(UMapView map);
List UMapView_getMeta(UMapView map);
um_fp UMapView_getKeyAtIndex(UMapView map, unsigned int index);
um_fp UMapView_getValAtIndex(UMapView map, unsigned int index);
um_fp UMapView_getValAtKey(UMapView map, um_fp key);
void UMapView_free(UMapView umv);
void UMapListView_free(UMapListView umv);

stringListView UMapListView_getVals(UMapListView map);
List UMapListView_getMeta(UMapListView map);
um_fp UMapListView_getVal(UMapListView map, unsigned int index);

REGISTER_SPECIAL_PRINTER("UMap", UMap, {
  List *metaList = in.metadata;
  stringList *keys = in.keys;
  stringList *vals = in.vals;
  put("{", 1);
  for (int i = 0; i < metaList->length; i++) {
    UMap_innertype type = mList_get(metaList, UMap_innertype, i);
    USETYPEPRINTER(um_fp, stringList_get(keys, i));
    put(":", 1);
    um_fp val = stringList_get(vals, i);
    switch (type) {
    case NORMAL:
      USETYPEPRINTER(um_fp, val);
      put(";", 1);

      break;
    case LIST:
      USENAMEDPRINTER("UMapListView", ((UMapListView){.raw = val}))
      break;
    case MAP:
      USENAMEDPRINTER("UMapView", ((UMapListView){.raw = val}))
      break;
    }
  }

  put("}", 1);
});
REGISTER_SPECIAL_PRINTER("UMapView", UMapView, {
  List metaList = UMapView_getMeta(in);
  stringListView keys = UMapView_getKeys(in);
  stringListView vals = UMapView_getVals(in);
  UMap_innertype *meta = (UMap_innertype *)metaList.head;
  put("{", 1);
  for (int i = 0; i < metaList.length; i++) {
    UMap_innertype type = meta[i];
    USETYPEPRINTER(um_fp, stringListView_get(keys, i));
    put(":", 1);
    um_fp val = stringListView_get(vals, i);
    switch (type) {
    case NORMAL:
      USETYPEPRINTER(um_fp, val);
      put(";", 1);

      break;
    case LIST:
      USENAMEDPRINTER("UMapListView", ((UMapListView){.raw = val}))
      break;
    case MAP:
      USENAMEDPRINTER("UMapView", ((UMapListView){.raw = val}))
      break;
    }
  }

  put("}", 1);
})
REGISTER_SPECIAL_PRINTER("UMapList", UMapList, {
  stringList *slv = in.vals;
  List *metaList = in.metadata;
  UMap_innertype *meta = (UMap_innertype *)metaList->head;
  USETYPEPRINTER(um_fp, um_from("["));
  for (int i = 0; i < metaList->length; i++) {
    UMap_innertype type = meta[i];
    um_fp val = stringList_get(slv, i);
    switch (type) {
    case NORMAL:
      USETYPEPRINTER(um_fp, val);
      break;
    case LIST:
      USENAMEDPRINTER("UMapListView", ((UMapListView){.raw = val}))
      break;
    case MAP:
      USENAMEDPRINTER("UMapView", ((UMapListView){.raw = val}))
      break;
    }
    if (i + 1 != metaList->length)
      USETYPEPRINTER(um_fp, um_from(","));
  }
  USETYPEPRINTER(um_fp, um_from("]"));
})
REGISTER_SPECIAL_PRINTER("UMapListView", UMapListView, {
  stringListView slv = UMapListView_getVals(in);
  List metaList = UMapListView_getMeta(in);
  UMap_innertype *meta = (UMap_innertype *)metaList.head;
  USETYPEPRINTER(um_fp, um_from("["));
  for (int i = 0; i < metaList.length; i++) {
    UMap_innertype type = meta[i];
    um_fp val = stringListView_get(slv, i);
    switch (type) {
    case NORMAL:
      USETYPEPRINTER(um_fp, val);
      break;
    case LIST:
      USENAMEDPRINTER("UMapListView", ((UMapListView){.raw = val}))
      break;
    case MAP:
      USENAMEDPRINTER("UMapView", ((UMapListView){.raw = val}))
      break;
    }
    if (i + 1 != metaList.length)
      USETYPEPRINTER(um_fp, um_from(","));
  }
  USETYPEPRINTER(um_fp, um_from("]"));
})

REGISTER_SPECIAL_PRINTER("UMap*", UMap *, { USENAMEDPRINTER("UMap", *in); });
REGISTER_SPECIAL_PRINTER("UMapList*", UMapList *,
                         { USENAMEDPRINTER("UMapList", *in); });

#endif // UMAP_H
