#include "umap.h"
#include "../string-List/stringList.h"
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
UMap *UMap_new() {
  UMap *res = (UMap *)malloc(sizeof(UMap));
  *res = (UMap){
      .keys = stringList_new(),
      .vals = stringList_new(),
      .metadata = List_new(sizeof(UMap_innertype)),
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
  unsigned int index = UMap_binarySearch(map, key);
  unsigned int length = stringList_length(map->keys);

  if (index < length && !um_fp_cmp(UMap_getKeyAtIndex(map, index), key)) {
    stringList_set(map->keys, key, index);
    stringList_set(map->vals, val, index);
    List_set(map->metadata, index, (UMap_innertype[1]){NORMAL});
  } else {
    stringList_insert(map->keys, key, index);
    stringList_insert(map->vals, val, index);
    List_insert(map->metadata, index, (UMap_innertype[1]){NORMAL});
  }
  return index;
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

void UMap_setChild(UMap *map, um_fp key, UMap_innertype type, UMap *ref) {
  um_fp um = UMap_toBuf(ref).raw;
  unsigned int index = UMap_set(map, key, um);
  List_set(map->metadata, index, &type);

  free(um.ptr);
}
void UMap_free(UMap *map) {
  stringList_free(map->keys);
  stringList_free(map->vals);
  List_free(map->metadata);
  free(map);
}
UMap *UMap_remake(UMap *map) {
  UMap *res = UMap_new();
  for (unsigned int i = 0; i < stringList_length(map->keys); i++) {
    um_fp key = UMap_getKeyAtIndex(map, i);
    um_fp val = UMap_getValAtIndex(map, i);
    if (key.ptr && key.width && val.ptr && val.width)
      UMap_set(res, key, val);
  }
  return res;
}

void UMapView_free(UMapView umv) { free(umv.raw.ptr); }
