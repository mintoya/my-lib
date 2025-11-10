#ifndef UMAP_H
#define UMAP_H

#include "allocator.h"
#include "my-list.h"
#include "print.h"
#include "stringList.h"
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

static inline size_t UMap_footprint(UMap *um) {
  return List_fullHeadArea(um->metadata) + stringList_footprint(um->keys) +
         stringList_footprint(um->vals);
}
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
UMap *UMap_new(const My_allocator *);
UMapList *UMapList_new(const My_allocator *);

um_fp UMap_getValAtKey(UMap *map, um_fp key);
static inline um_fp UMap_get(UMap *map, um_fp key) {
  return UMap_getValAtKey(map, key);
}
unsigned int UMap_set(UMap *map, um_fp key, um_fp val);
UMap *UMap_remake(UMap *map);
unsigned int UMap_setChild(UMap *map, um_fp key, UMap *ref);
unsigned int UMap_setList(UMap *map, um_fp key, UMapList *ref);
unsigned int UMapList_setChild(UMapList *map, unsigned int key, UMap *ref);
unsigned int UMapList_setList(UMapList *map, unsigned int key, UMapList *ref);
static inline unsigned int UMapList_appendChild(UMapList *map, UMap *ref) {
  return UMapList_setChild(map, map->metadata->length, ref);
}
static inline unsigned int UMapList_appendList(UMapList *map, UMapList *ref) {
  return UMapList_setList(map, map->metadata->length, ref);
}

static inline void UMap_free(UMap *map) {
  const My_allocator *allocator = map->metadata->allocator;
  stringList_free(map->keys);
  stringList_free(map->vals);
  List_free(map->metadata);
  allocator->free(map);
}
static inline void UMapList_free(UMapList *map) {
  const My_allocator *allocator = map->metadata->allocator;
  stringList_free(map->vals);
  List_free(map->metadata);
  allocator->free(map);
}
/*
 * not modifiable
 * freeing keyReg itself deletes this
 */
UMapView UMap_toBuf(UMap *map);
UMapListView UMapList_toBuf(UMapList *map);
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
  PUTS("{", 1);
  for (int i = 0; i < metaList->length; i++) {
    UMap_innertype type = mList_get(metaList, UMap_innertype, i);
    USETYPEPRINTER(um_fp, stringList_get(keys, i));
    PUTS(":", 1);
    um_fp val = stringList_get(vals, i);
    switch (type) {
    case NORMAL:
      USETYPEPRINTER(um_fp, val);
      PUTS(";", 1);

      break;
    case LIST:
      USENAMEDPRINTER("UMapListView", ((UMapListView){.raw = val}))
      break;
    case MAP:
      USENAMEDPRINTER("UMapView", ((UMapListView){.raw = val}))
      break;
    }
  }

  PUTS("}", 1);
});
REGISTER_SPECIAL_PRINTER("UMapView", UMapView, {
  List metaList = UMapView_getMeta(in);
  stringListView keys = UMapView_getKeys(in);
  stringListView vals = UMapView_getVals(in);
  UMap_innertype *meta = (UMap_innertype *)metaList.head;
  PUTS("{", 1);
  for (int i = 0; i < metaList.length; i++) {
    UMap_innertype type = meta[i];
    USETYPEPRINTER(um_fp, stringListView_get(keys, i));
    PUTS(":", 1);
    um_fp val = stringListView_get(vals, i);
    switch (type) {
    case NORMAL:
      USETYPEPRINTER(um_fp, val);
      PUTS(";", 1);

      break;
    case LIST:
      USENAMEDPRINTER("UMapListView", ((UMapListView){.raw = val}))
      break;
    case MAP:
      USENAMEDPRINTER("UMapView", ((UMapListView){.raw = val}))
      break;
    }
  }

  PUTS("}", 1);
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
// made to mimic the setup the "kml" parser  supports

static inline void UMap_cleanup_handler(UMap **um) {
  if (um && *um)
    UMap_free(*um);
  *um = NULL;
}
static inline void UMapList_cleanup_handler(UMapList **um) {
  if (um && *um)
    UMapList_free(*um);
  *um = NULL;
}

#define UMap_scoped UMap __attribute__((cleanup(UMap_cleanup_handler)))
#define UMapList_scoped                                                        \
  UMapList __attribute__((cleanup(UMapList_cleanup_handler)))

#endif // UMAP_H
#ifdef UMAP_C
#include "stringList.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// static unsigned int runs = 0;
unsigned int UMap_linearSearch(UMap *map, um_fp key) {
  unsigned int res = 0;
  unsigned int len = stringList_length(map->keys);
  while (res < len && um_fp_cmp(key, UMap_getKeyAtIndex(map, res)) > 0) {
    res++;
  }
  return res;
}
unsigned int UMapView_binarySearch(UMapView map, um_fp key) {
  stringListView keys = UMapView_getKeys(map);
  unsigned int length = stringListView_MetaList(keys).length;
  unsigned int region = length;
  unsigned int front = 0;

  while (region > 2) {
    char less =
        um_fp_cmp(key, UMapView_getKeyAtIndex(map, front + region / 2)) > 0;
    region /= 2;
    if (less)
      front += region;
  }

  unsigned int res = front;
  while (res < length && um_fp_cmp(key, UMapView_getKeyAtIndex(map, res)) > 0) {
    res++;
  }
  return res;
}
UMap *UMap_new(const My_allocator *allocator) {
  UMap *res = (UMap *)allocator->alloc(sizeof(UMap));
  *res = (UMap){
      .keys = stringList_new(allocator),
      .vals = stringList_new(allocator),
      .metadata = List_new(allocator, sizeof(UMap_innertype)),
  };
  return res;
}
UMapList *UMapList_new(const My_allocator *allocator) {
  UMapList *res = (UMapList *)malloc(sizeof(UMapList));
  *res = (UMapList){
      .vals = stringList_new(allocator),
      .metadata = List_new(allocator, sizeof(UMap_innertype)),
  };
  return res;
}

um_fp UMap_getValAtKey(UMap *map, um_fp key) {
  unsigned int index = UMap_binarySearch(map, key);
  um_fp res = nullUmf;
  um_fp temp = UMap_getKeyAtIndex(map, index);
  int cmp = um_fp_cmp(key, temp);
  if (!cmp) {
    res = stringList_get(map->vals, index);
  }
  return res;
}
um_fp UMapList_get(UMapList *map, unsigned int key) {
  return stringList_get(map->vals, key);
}

unsigned int UMap_binarySearch(UMap *map, um_fp key) {
  unsigned int region = stringList_length(map->keys);
  unsigned int front = 0;

  while (region > 2) {
    char less = um_fp_cmp(key, UMap_getKeyAtIndex(map, front + region / 2)) > 0;
    region /= 2;
    if (less)
      front += region;
  }

  unsigned int res = front;
  while (res < stringList_length(map->keys) &&
         um_fp_cmp(key, UMap_getKeyAtIndex(map, res)) > 0) {
    res++;
  }
  return res;
}
unsigned int UMap_set(UMap *map, um_fp key, um_fp val) {
  if (!key.width)
    return -1;
  unsigned int index = UMap_binarySearch(map, key);
  unsigned int length = map->metadata->length;

  if (index < length && !um_fp_cmp(UMap_getKeyAtIndex(map, index), key)) {
    stringList_set(map->keys, key, index);
    stringList_set(map->vals, val, index);
    mList_set(map->metadata, UMap_innertype, NORMAL, index);
  } else {
    stringList_insert(map->keys, key, index);
    stringList_insert(map->vals, val, index);
    mList_insert(map->metadata, UMap_innertype, NORMAL, index);
  }
  return index;
};
unsigned int UMapList_set(UMapList *map, unsigned int index, um_fp val) {
  while (index >= map->metadata->length) {
    stringList_append(map->vals, nullUmf);
    mList_add(map->metadata, UMap_innertype, NORMAL);
  }
  stringList_set(map->vals, val, index);
  mList_set(map->metadata, UMap_innertype, NORMAL, index);
  return index;
};
unsigned int UMapList_append(UMapList *map, um_fp val) {
  stringList_append(map->vals, val);
  mList_add(map->metadata, UMap_innertype, NORMAL);
  return map->metadata->length - 1;
};

#define advance(dst, src, length)                                              \
  memcpy(dst, src, length);                                                    \
  dst += length / sizeof(uint8_t);
// memory layout:
//  { keysSize|valsSize|metaSize|keys|vals|meta }
UMapView UMap_toBuf(UMap *map) {
  stringListView kb = stringList_tobuf(map->keys);
  stringListView vb = stringList_tobuf(map->vals);
  um_fp keys = kb.raw;
  um_fp vals = vb.raw;
  um_fp meta = (um_fp){
      .ptr = map->metadata->head,
      .width = List_headArea(map->metadata),
  };
  size_t finalWidth = keys.width + vals.width + meta.width + 3 * sizeof(size_t);
  um_fp res = (um_fp){
      .ptr = (uint8_t *)malloc(finalWidth),
      .width = finalWidth,
  };
  uint8_t *ptr = res.ptr;

  advance(ptr, &(keys.width), sizeof(size_t));
  advance(ptr, &(vals.width), sizeof(size_t));
  advance(ptr, &(meta.width), sizeof(size_t));

  advance(ptr, keys.ptr, keys.width);
  advance(ptr, vals.ptr, vals.width);
  advance(ptr, meta.ptr, meta.width);

  stringListView_free(kb);
  stringListView_free(vb);

  return (UMapView){.raw = res};
}
// memory layout:
//  { valsSize|metaSize|vals|meta }
UMapListView UMapList_toBuf(UMapList *map) {
  stringListView vb = stringList_tobuf(map->vals);
  um_fp vals = vb.raw;
  um_fp meta = (um_fp){
      .ptr = map->metadata->head,
      .width = List_headArea(map->metadata),
  };
  size_t finalWidth = vals.width + meta.width + 3 * sizeof(size_t);
  um_fp res = (um_fp){
      .ptr = (uint8_t *)malloc(finalWidth),
      .width = finalWidth,
  };
  uint8_t *ptr = res.ptr;

  advance(ptr, &(vals.width), sizeof(size_t));
  advance(ptr, &(meta.width), sizeof(size_t));

  advance(ptr, vals.ptr, vals.width);
  advance(ptr, meta.ptr, meta.width);

  stringListView_free(vb);
  return (UMapListView){.raw = res};
}
#undef advance

List UMapView_getMeta(UMapView map) {
  List res;
  res.width = sizeof(UMap_innertype);
  size_t *sizes = ((size_t *)map.raw.ptr);
  res.length = sizes[2] / sizeof(UMap_innertype);
  res.size = res.length;
  res.head = map.raw.ptr + sizeof(size_t) * 3 + sizes[0] + sizes[1];
  return res;
}
stringListView UMapView_getVals(UMapView map) {
  stringListView res;
  size_t *sizes = ((size_t *)map.raw.ptr);
  uint8_t *place = map.raw.ptr + sizeof(size_t) * 3 + sizes[0];
  res.raw = (um_fp){.ptr = place, .width = sizes[1]};
  return res;
}
stringListView UMapView_getKeys(UMapView map) {
  stringListView res;
  size_t size = *(size_t *)map.raw.ptr;
  uint8_t *place = map.raw.ptr + sizeof(size_t) * 3;
  res.raw = (um_fp){.ptr = place, .width = size};
  return res;
}
um_fp UMapView_getKeyAtIndex(UMapView map, unsigned int index) {
  stringListView sl = UMapView_getKeys(map);
  List meta = stringListView_MetaList(sl);
  return stringListView_get(sl, index);
};
um_fp UMapView_getValAtIndex(UMapView map, unsigned int index) {
  stringListView sl = UMapView_getVals(map);
  List meta = stringListView_MetaList(sl);
  return stringListView_get(sl, index);
};

um_fp UMapView_getValAtKey(UMapView map, um_fp key) {
  unsigned int index = UMapView_binarySearch(map, key);
  um_fp res = nullUmf;
  int cmp = um_fp_cmp(key, UMapView_getKeyAtIndex(map, index));
  if (!cmp) {
    res = stringListView_get(UMapView_getVals(map), index);
  }
  return res;
}

unsigned int UMap_setChild(UMap *map, um_fp key, UMap *ref) {
  if (!key.width)
    return -1;
  um_fp um = UMap_toBuf(ref).raw;
  unsigned int index = UMap_set(map, key, um);
  mList_set(map->metadata, UMap_innertype, MAP, index);
  free(um.ptr);
  return index;
}

unsigned int UMapList_setChild(UMapList *map, unsigned int key, UMap *ref) {
  um_fp um = UMap_toBuf(ref).raw;
  unsigned int index = UMapList_set(map, key, um);
  mList_set(map->metadata, UMap_innertype, MAP, index);
  free(um.ptr);
  return index;
}
unsigned int UMapList_setList(UMapList *map, unsigned int key, UMapList *ref) {
  um_fp um = UMapList_toBuf(ref).raw;
  unsigned int index = UMapList_set(map, key, um);
  mList_set(map->metadata, UMap_innertype, LIST, index);
  free(um.ptr);
  return index;
}
unsigned int UMap_setList(UMap *map, um_fp key, UMapList *ref) {
  if (!key.width)
    return -1;
  um_fp um = UMapList_toBuf(ref).raw;
  unsigned int index = UMap_set(map, key, um);
  mList_set(map->metadata, UMap_innertype, LIST, index);
  free(um.ptr);
  return index;
}

UMap *UMap_remake(UMap *map) {
  UMap *res = UMap_new(map->metadata->allocator);
  for (unsigned int i = 0; i < stringList_length(map->keys); i++) {
    um_fp key = UMap_getKeyAtIndex(map, i);
    um_fp val = UMap_getValAtIndex(map, i);
    if (key.ptr && key.width && val.ptr && val.width)
      UMap_set(res, key, val);
  }
  return res;
}

//  { valsSize|metaSize|vals|meta }
stringListView UMapListView_getVals(UMapListView map) {
  stringListView res;
  size_t *sizes = ((size_t *)map.raw.ptr);
  uint8_t *place = map.raw.ptr + sizeof(size_t) * 2;
  res.raw = (um_fp){.ptr = place, .width = sizes[0]};
  return res;
}

List UMapListView_getMeta(UMapListView map) {
  List res;
  res.width = sizeof(UMap_innertype);
  size_t *sizes = ((size_t *)map.raw.ptr);
  res.length = sizes[1] / sizeof(UMap_innertype);
  res.size = res.length;
  res.head = map.raw.ptr + sizeof(size_t) * 2 + sizes[0];
  return res;
}
um_fp UMapListView_getVal(UMapListView map, unsigned int index) {
  return stringListView_get(UMapListView_getVals(map), index);
}

void UMapView_free(UMapView umv) { free(umv.raw.ptr); }
void UMapListView_free(UMapListView umv) { free(umv.raw.ptr); }
#endif // UMAP_C
