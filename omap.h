#ifndef OMAP_H
#define OMAP_H
#include "allocator.h"
#include "fptr.h"
#include "stringList.h"
#include "umap.h"
#include <string.h>

typedef enum : uint8_t {
  RAW = 0,
  SLIST,
  OMAP,
} OMap_Meta;
typedef struct {
  fptr k;
  fptr v;
  OMap_Meta t;
} OMap_both;
typedef struct {
  fptr v;
  OMap_Meta t;
} OMap_V;

const OMap_Meta OMap_Meta_RAW = (OMap_Meta){RAW};
const OMap_Meta OMap_Meta_SLIST = (OMap_Meta){SLIST};
const OMap_Meta OMap_Meta_OMAP = (OMap_Meta){OMAP};
typedef struct {
  stringList *KVs;
} OMap;

static inline fptr OMap_toView(const My_allocator *allocator, OMap *map) { return stringList_toView(allocator, map->KVs).raw; }

static inline fptr OMap_getKey(const OMap *map, unsigned int index) { return stringList_get(map->KVs, index * 2); }
static inline fptr OMap_getVal(const OMap *map, unsigned int index) { return stringList_get(map->KVs, index * 2 + 1); }
static inline int OMap_length(const OMap *map) { return stringList_length(map->KVs) / 2; }

OMap_V OMap_get(OMap *map, fptr key);
OMap_both OMap_getBoth(const OMap *map, fptr key);
typedef fptr OMapView;
unsigned int OMap_set(OMap *map, fptr key, fptr val);
// takes fptr*,OMap*,stringList*
unsigned int OMap_setKind(OMap *map, fptr key, fptr val, OMap_Meta kind);
static inline void OMap_setObj(OMap *map, fptr key, void *val, OMap_Meta kind) {
  fptr fval;
  switch (kind) {
  case OMAP:
    fval =
        OMap_toView(&defaultAllocator, (OMap *)val);
    break;
  case RAW:
    fval = *(fptr *)val;
    break;
  case SLIST:
    fval = stringList_toView(&defaultAllocator, (stringList *)val).raw;
    break;
  }

  unsigned int index = OMap_setKind(map, key, fval, kind);

  switch (kind) {
  case RAW:
    break;
  default:
    aFree(&defaultAllocator, fval.ptr);
    break;
  }
}
// takes fptr*,OMap*,stringList*
static inline void StringList_setObj(stringList *list, unsigned int key, void *val, OMap_Meta kind) {
  fptr fval;
  switch (kind) {
  case OMAP:
    fval =
        OMap_toView(&defaultAllocator, (OMap *)val);
    break;
  case RAW:
    fval = *(fptr *)val;
    break;
  case SLIST:
    fval = stringList_toView(&defaultAllocator, (stringList *)val).raw;
    break;
  }
  fptr both;
  both.width = sizeof(OMap_Meta) + fval.width;
  uint8_t *dataBuffer = (uint8_t *)aAlloc(&defaultAllocator, both.width);
  both.ptr = dataBuffer;
  *(OMap_Meta *)dataBuffer = kind;
  memcpy(dataBuffer + sizeof(OMap_Meta), fval.ptr, fval.width);
  stringList_set(list, both, key);
  aFree(&defaultAllocator, both.ptr);
  switch (kind) {
  case RAW:
    break;
  default:
    aFree(&defaultAllocator, fval.ptr);
    break;
  }
}
static inline OMap_V Stringlist_getObj(stringList *list, unsigned int key) {
  fptr val = stringList_get(list, key);
  if (!val.ptr)
    return (OMap_V){
        .v = nullFptr,
        .t = RAW,
    };
  OMap_V res = (OMap_V){.t = *(OMap_Meta *)(val.ptr)};
  res.v = (fptr){
      .width = val.width - sizeof(OMap_Meta),
      .ptr = val.ptr + sizeof(OMap_Meta),
  };
  return res;
}
static inline OMap_V StringlistView_getObj(stringListView list, unsigned int key) {
  fptr val = stringListView_get(list, key);
  if (!val.ptr)
    return (OMap_V){
        .v = nullFptr,
        .t = RAW,
    };
  OMap_V res = (OMap_V){.t = *(OMap_Meta *)(val.ptr)};
  res.v = (fptr){
      .width = val.width - sizeof(OMap_Meta),
      .ptr = val.ptr + sizeof(OMap_Meta),
  };
  return res;
}
static inline void StringList_appendObj(stringList *list, void *val, OMap_Meta kind) {
  fptr fval;
  switch (kind) {
  case OMAP:
    fval =
        OMap_toView(&defaultAllocator, (OMap *)val);
    break;
  case RAW:
    fval = *(fptr *)val;
    break;
  case SLIST:
    fval = stringList_toView(&defaultAllocator, (stringList *)val).raw;
    break;
  }
  fptr both;
  both.width = sizeof(OMap_Meta) + fval.width;
  uint8_t *dataBuffer = (uint8_t *)aAlloc(&defaultAllocator, both.width);
  both.ptr = dataBuffer;
  *(OMap_Meta *)dataBuffer = kind;
  memcpy(dataBuffer + sizeof(OMap_Meta), fval.ptr, fval.width);
  stringList_append(list, both);
  aFree(&defaultAllocator, both.ptr);
  switch (kind) {
  case RAW:
    break;
  default:
    aFree(&defaultAllocator, fval.ptr);
    break;
  }
}

OMap *OMap_new(const My_allocator *allocator);

unsigned int OMap_binarySearch(const OMap *map, fptr key);
typedef struct {
  stringList_Solid KVs;
} OMap_solid;
static inline unsigned int OMapView_length(const OMapView omv) {
  return stringListView_length((stringListView){omv}) / 2;
}
OMap_both OMapView_get(OMapView mv, fptr key);
OMap_both OMapView_getIndex(OMapView mv, unsigned int index);
static inline OMap_solid OMap_fromView(const OMapView omv) { return (OMap_solid){stringList_fromView((stringListView){omv})}; }

static inline size_t OMap_footprint(OMap *map) { return stringList_footprint(map->KVs); }
static inline void OMap_free(OMap *map) {
  const My_allocator *allocator = map->KVs->List_char.allocator;
  stringList_free(map->KVs);
  aFree(allocator, map);
}
static inline void OMapView_free(const My_allocator *allocator, OMapView v) { aFree(allocator, v.ptr); }

//
//
//
//
//
//
// printers

#include "print.h"
REGISTER_SPECIAL_PRINTER("OMap_stringlistView", stringListView, {
  PUTS("[", 1);
  const unsigned int length = stringListView_length(in);
  for (int i = 0; i < length; i++) {
    OMap_V el = StringlistView_getObj(in, i);
    switch (el.t) {
    case RAW:
      USETYPEPRINTER(fptr, el.v);
    PUTS(",", 1);
      break;
    case OMAP:
      USENAMEDPRINTER("OMapView", el.v);
      break;
    case SLIST:
      USENAMEDPRINTER("OMap_stringlistView", el.v);
      break;
    }
  }
  PUTS("]", 1);
});
REGISTER_PRINTER(OMapView, {
  const int length = OMapView_length(in);
  PUTS("{", 1);
  for (uint i = 0; i < length; i++) {
    OMap_both both = OMapView_getIndex(in, i);
    USETYPEPRINTER(fptr, both.k);
    PUTS(":", 1);
    switch (both.t) {
    case RAW:
      USETYPEPRINTER(fptr, both.v);
      PUTS(";", 1);
      break;
    case OMAP:
      USETYPEPRINTER(OMapView, both.v);
      break;
    case SLIST:
      USENAMEDPRINTER("OMap_stringlistView", both.v);
      break;
    }
  }
  PUTS("}", 1);
});
REGISTER_PRINTER(OMap, {
  const int length = OMap_length(&in);
  PUTS("{", 1);
  for (uint i = 0; i < length; i++) {
    OMap_both both;
    both.k = OMap_getKey(&in, i);
    OMap_V m = OMap_get(&in, both.k);
    both.t = m.t;
    both.v = m.v;
    USETYPEPRINTER(fptr, both.k);
    PUTS(":", 1);
    switch (both.t) {
    case RAW:
      USETYPEPRINTER(fptr, both.v);
      PUTS(";", 1);
      break;
    case OMAP:
      USETYPEPRINTER(OMapView, both.v);
      break;
    case SLIST:
      USENAMEDPRINTER("OMap_stringlistView", both.v);
      break;
    }
  }
  PUTS("}", 1);
})

#endif // OMAP_H
#ifdef OMAP_C
OMap *OMap_new(const My_allocator *allocator) {
  OMap *res = (OMap *)aAlloc(allocator, sizeof(OMap));
  res->KVs = stringList_new(allocator);
  return res;
}
unsigned int OMap_binarySearch(const OMap *map, fptr key) {
  unsigned int region = OMap_length(map);
  unsigned int front = 0;
  while (region > 2) {
    char less = fptr_cmp(key, OMap_getKey(map, front + region / 2)) > 0;
    region /= 2;
    if (less)
      front += region;
  }
  unsigned int res = front;
  while (res < OMap_length(map) &&
         fptr_cmp(key, OMap_getKey(map, res)) > 0) {
    res++;
  }
  return res;
}
OMap_V OMap_get(OMap *map, fptr key) {
  unsigned int index = OMap_binarySearch(map, key);
  if (index < OMap_length(map)) {
    fptr vall = OMap_getVal(map, index);
    fptr res = (fptr){vall.width - sizeof(OMap_Meta), vall.ptr + sizeof(OMap_Meta)};
    return (OMap_V){res, *(OMap_Meta *)(vall.ptr)};
  } else
    return (OMap_V){nullFptr, RAW};
}
OMap_both OMap_getBoth(const OMap *map, fptr key) {
  unsigned int index = OMap_binarySearch(map, key);
  if (index < OMap_length(map)) {
    fptr vall = OMap_getVal(map, index);
    fptr res = (fptr){vall.width - sizeof(OMap_Meta), vall.ptr + sizeof(OMap_Meta)};
    return (OMap_both){OMap_getKey(map, index), res, *(OMap_Meta *)(vall.ptr)};
  } else
    return (OMap_both){nullFptr, nullFptr, RAW};
}
#define STACKMAX (sizeof(intmax_t) * 5)
unsigned int OMap_set(OMap *map, fptr key, fptr val) {
  unsigned int index = OMap_binarySearch(map, key);
  fptr both = (fptr){
      .width = sizeof(OMap_Meta) + val.width
  };
  uint8_t *dataBuffer;
  if (both.width > STACKMAX) {
    dataBuffer = (uint8_t *)alloca(both.width);
  } else {
    dataBuffer = (uint8_t *)aAlloc(map->KVs->List_char.allocator, both.width);
  }
  *(OMap_Meta *)dataBuffer = OMap_Meta_RAW;
  memcpy(dataBuffer + sizeof(OMap_Meta), val.ptr, val.width);
  both.ptr = dataBuffer;
  if (fptr_eq(key, OMap_getKey(map, index))) {
    stringList_set(map->KVs, both, index * 2 + 1);
  } else {
    stringList_insert(map->KVs, both, index * 2);
    stringList_insert(map->KVs, key, index * 2);
  }
  if (both.width < STACKMAX) {
    aFree(map->KVs->List_char.allocator, both.ptr);
  }
  return index;
}
unsigned int OMap_setKind(OMap *map, fptr key, fptr val, OMap_Meta kind) {
  unsigned int index = OMap_binarySearch(map, key);
  fptr both = (fptr){
      .width = sizeof(OMap_Meta) + val.width
  };
  uint8_t *dataBuffer;
  if (both.width > STACKMAX) {
    dataBuffer = (uint8_t *)alloca(both.width);
  } else {
    dataBuffer = (uint8_t *)aAlloc(map->KVs->List_char.allocator, both.width);
  }
  *(OMap_Meta *)dataBuffer = kind;
  memcpy(dataBuffer + sizeof(OMap_Meta), val.ptr, val.width);
  both.ptr = dataBuffer;
  if (fptr_eq(key, OMap_getKey(map, index))) {
    stringList_set(map->KVs, both, index * 2 + 1);
  } else {
    stringList_insert(map->KVs, both, index * 2);
    stringList_insert(map->KVs, key, index * 2);
  }
  if (both.width < STACKMAX) {
    aFree(map->KVs->List_char.allocator, both.ptr);
  }
  return index;
}
OMap_both OMapView_getIndex(OMapView mv, unsigned int index) {
  unsigned int length = OMapView_length(mv);
  OMap_solid map = OMap_fromView(mv);
  if (index > length)
    return (OMap_both){nullFptr, nullFptr, RAW};
  stringMetaData thisKey = map.KVs.Arr_stringMetaData[index * 2];
  stringMetaData thisVal = map.KVs.Arr_stringMetaData[index * 2 + 1];
  fptr vall = (fptr){thisVal.width, map.KVs.Arr_char + thisVal.index};

  fptr key = (fptr){thisKey.width, map.KVs.Arr_char + thisKey.index};
  fptr val = (fptr){vall.width - sizeof(OMap_Meta), vall.ptr + sizeof(OMap_Meta)};
  OMap_Meta ttype = *(OMap_Meta *)(vall.ptr);
  return (OMap_both){
      .k = key,
      .v = val,
      .t = ttype,
  };
}
OMap_both OMapView_get(OMapView mv, fptr key) {
  unsigned int region = OMapView_length(mv);
  unsigned int front = 0;
  while (region > 2) {
    char less = fptr_cmp(key, OMapView_getIndex(mv, front + region / 2).k) > 0;
    region /= 2;
    if (less)
      front += region;
  }
  unsigned int res = front;
  while (res < OMapView_length(mv) &&
         fptr_cmp(key, OMapView_getIndex(mv, res).k) > 0) {
    res++;
  }
  return OMapView_getIndex(mv, res);
}
#undef STACKMAX
#endif // OMAP_C
