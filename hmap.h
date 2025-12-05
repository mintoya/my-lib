#ifndef HMAP_H
#define HMAP_H
#include "allocator.h"
#include "fptr.h"
#include "stringList.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct HMap HMap;

extern inline umax HMap_hash(const fptr str);

// TODO handle errors
// pub extern fn HMap_new(*const c.My_allocator, u32) *HMap;
// pub extern fn HMap_free(arg: *HMap) void;
// pub extern fn HMap_remake(arg: *HMap) void;
// pub extern fn HMap_get(map: *HMap, c.fptr) c.fptr;
// pub extern fn HMap_set(map: *HMap, key: c.fptr, val: c.fptr) c_uint;

// exports
HMap *HMap_new(const My_allocator *allocator, unsigned int metaSize);
void HMap_free(HMap *hm);
void HMap_remake(HMap *hm);
fptr HMap_get(HMap *, fptr);
u32 HMap_set(HMap *map, fptr key, fptr val);
u32 HMap_count(const HMap *map);
extern inline fptr HMap_getKey(const HMap *map, u32 n); // linked to getKey, but otherwise meaningless
extern inline fptr HMap_getVal(const HMap *map, u32 n); // linked to getVal, but otherwise meaningless

extern inline void HMap_preload(HMap *, u32 pairCount, u32 guessSize);
stringList *HMap_getkeys(HMap *map);
size_t HMap_countCollisions(HMap *map);
typedef struct HMap_both {
  fptr key;
  fptr val;
} HMap_both;
HMap_both HMap_getBoth(HMap *map, fptr key);
HMap_both HMap_getNth(const HMap *map, u32 n);

size_t HMap_footprint(HMap *hm);
static inline void HMap_cleanup_handler(HMap **hm) {
  if (hm && *hm) {
    HMap_free(*hm);
    *hm = NULL;
  }
}
#define HMap_scoped [[gnu::cleanup(HMap_cleanup_handler)]] HMap

#endif // HMAP_H

#ifdef HMAP_C

typedef struct {
  u32 index;
  u32 next;
  bool hasindex : 1;
  bool hasnext : 1;
} HMap_innertype;

static const HMap_innertype HMap_innerEmpty = (HMap_innertype){0, 0, 0, 0};

typedef struct HMap {
  unsigned int metaSize;
  HMap_innertype *metadata;
  List *links;
  stringList *KVs;
} HMap;
size_t HMap_footprint(HMap *hm) {
  size_t res = stringList_footprint(hm->KVs) +
               hm->metaSize * sizeof(HMap_innertype) +
               List_fullHeadArea(hm->links);
  return res;
}
stringList *HMap_getkeys(HMap *map) {
  stringList *keys = stringList_new(map->KVs->List_stringMetaData.allocator);

  for (size_t i = 0; i < map->metaSize; i++) {
    HMap_innertype *h = &map->metadata[i];

    while (h->hasindex) {
      stringList_append(keys, stringList_get(map->KVs, h->index));

      if (!h->hasnext)
        break;

      h = (HMap_innertype *)List_getRef(map->links, h->next);
      if (!h)
        break;
    }
  }

  return keys;
}
HMap_both HMap_getNth(const HMap *map, u32 n) {
  return (HMap_both){
      .key = stringList_get(map->KVs, n * 2),
      .val = stringList_get(map->KVs, n * 2 + 1),
  };
}
u32 HMap_count(const HMap *map) {
  return stringList_length(map->KVs) / 2;
}
size_t HMap_countCollisions(HMap *map) {
  return (map->links->length);
}
void HMap_remake(HMap *hm) {
  stringList *l = stringList_remake(hm->KVs);
  stringList_free(hm->KVs);
  hm->KVs = l;
}
#include <assert.h>
inline umax HMap_hash(const fptr str) {
  umax hash = 5381;
  size_t s = 0;
  for (; s < str.width; s++)
    hash = ((hash << 5) + hash) + (str.ptr[s]);

  return hash;

  // umax res = HMap_h;
  //
  // const size_t width = str.width;
  // const uint8_t *ptr = str.ptr;
  // const intmax_t *starta = (intmax_t *)ptr;
  // const size_t top = width / sizeof(intmax_t);
  // const size_t resta = top * sizeof(intmax_t);
  //
  // for (size_t i = 0; i < top; i++) {
  //   intmax_t chunk;
  //   memcpy(&chunk, ptr + i * sizeof(intmax_t), sizeof(intmax_t));
  //   res ^= (res << 10) + chunk;
  // }
  // for (size_t i = resta; i < width; i++)
  //   res ^= (res << 3) + ptr[i];
  //
  // return res;
}
void HMap_free(HMap *hm) {
  const My_allocator *allocator = hm->links->allocator;
  List_free(hm->links);
  stringList_free(hm->KVs);
  aFree(allocator, hm->metadata);
  aFree(allocator, hm);
}
HMap *HMap_new(const My_allocator *allocator, unsigned int metaSize) {
  assert(metaSize != 0 && "metaSize cant be 0");

  HMap *res = (HMap *)aAlloc(allocator, sizeof(HMap));
  *res = (HMap){
      .metaSize = metaSize,
      .metadata = (HMap_innertype *)aAlloc(allocator, metaSize * sizeof(HMap_innertype)),
      .links = List_new(allocator, sizeof(HMap_innertype)),
      .KVs = stringList_new(allocator),
  };
  List_resize(res->links, metaSize / 4);
  lmemset(res->metadata, metaSize, HMap_innerEmpty);
  return res;
}
unsigned int HMap_setForce(HMap *map, HMap_innertype *handle, fptr key, fptr val) {
  if (!handle->hasindex) {
    handle->index = stringList_append(map->KVs, key);
    handle->hasindex = 1;
    stringList_append(map->KVs, val);
    return handle->index;
  } else if (!fptr_cmp(stringList_get(map->KVs, handle->index), key)) {
    stringList_set(map->KVs, val, handle->index + 1);
    return handle->index;
  } else {
    // collisoin
    size_t next_index;
    if (!handle->hasnext) {
      handle->hasnext = 1;
      handle->next = map->links->length;
      next_index = handle->next;
      List_append(map->links, NULL);
    } else {
      next_index = handle->next;
    }
    return HMap_setForce(
        map, (HMap_innertype *)List_getRef(map->links, next_index), key, val
    );
  }
}
u32 HMap_set(HMap *map, fptr key, fptr val) {
  unsigned int hash = HMap_hash(key);
  if (map->metaSize == 0) {
    fprintf(stderr, "\n\nkvs: %p,links: %p,metaptr: %p,metasize: %u\n\n", map->KVs, map->links, map->metadata, map->metaSize);
    fflush(stderr);
  }
  assert(map->metaSize != 0 && "hmap corrupted?");
  HMap_innertype *ht = map->metadata + (hash % map->metaSize);
  return HMap_setForce(map, ht, key, val);
}
fptr HMap_get(HMap *map, fptr key) {
  unsigned int hash = HMap_hash(key);
  HMap_innertype *ht = map->metadata + (hash % map->metaSize);
  while (1) {
    if (!fptr_cmp(key, stringList_get(map->KVs, ht->index))) {
      return stringList_get(map->KVs, ht->index + 1);
    }
    if (!ht->hasnext)
      return nullFptr;
    ht = (HMap_innertype *)List_getRef(map->links, ht->next);
  }
}
struct HMap_both HMap_getBoth(HMap *map, fptr key) {
  unsigned int hash = HMap_hash(key);
  HMap_innertype *ht = map->metadata + (hash % map->metaSize);
  while (1) {
    if (!fptr_cmp(key, stringList_get(map->KVs, ht->index))) {
      return (struct HMap_both){
          .key = stringList_get(map->KVs, ht->index),
          .val = stringList_get(map->KVs, ht->index + 1),
      };
    }
    if (!ht->hasnext)
      return (struct HMap_both){nullFptr, nullFptr};
    ht = (HMap_innertype *)List_getRef(map->links, ht->next);
  }
}
inline void HMap_preload(HMap *map, u32 pairCount, u32 guessSize) {
  stringList_preload(map->KVs, pairCount * 2, guessSize);
}
inline fptr HMap_getKey(const HMap *map, u32 n) {
  return stringList_get(map->KVs, n * 2);
}
inline fptr HMap_getVal(const HMap *map, u32 n) {
  return stringList_get(map->KVs, n * 2 + 1);
}
#endif // HMAP_C
