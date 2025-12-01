#ifndef HMAP_H
#define HMAP_H
#include "allocator.h"
#include "fptr.h"
#include "stringList.h"
#include <stdint.h>
#include <stdlib.h>
typedef struct {
  unsigned int index;
  unsigned int next;
  bool hasindex : 1;
  bool hasnext : 1;
} HMap_innertype;
typedef struct {
  unsigned int metaSize;
  HMap_innertype *metadata;
  List *links;
  stringList *KVs;
} HMap;
static inline size_t HMap_footprint(HMap *hm) {
  size_t res = stringList_footprint(hm->KVs) +
               hm->metaSize * sizeof(HMap_innertype) +
               List_fullHeadArea(hm->links);
  return res;
}

#define HMap_innerEmpty ((HMap_innertype){0})
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// #TODO
// #include "komihash/komihash.h"
// [[gnu::pure]] static inline unsigned int HMap_hash(fptr str) {
//   return komihash(str.ptr, str.width, 6767);
// }

static const umax HMap_h = (0x67676141420);
[[gnu::pure]] static inline umax HMap_hash(const fptr str) {
  umax res = HMap_h;

  const size_t width = str.width;
  const uint8_t *ptr = str.ptr;
  const intmax_t *starta = (intmax_t *)ptr;
  const size_t top = width / sizeof(intmax_t);
  const size_t resta = top * sizeof(intmax_t);

  // for (size_t i = 0; i < top; i++)
  //   res ^= (res << 7) + starta[i];
  for (size_t i = 0; i < top; i++) {
    intmax_t chunk;
    memcpy(&chunk, ptr + i * sizeof(intmax_t), sizeof(intmax_t));
    res ^= (res << 10) + chunk;
  }
  for (size_t i = resta; i < width; i++)
    res ^= (res << 3) + ptr[i];

  return res;
}
static inline HMap *HMap_new(const My_allocator *allocator, unsigned int metaSize) {

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
unsigned int HMap_set(HMap *, fptr key, fptr val);
fptr HMap_get(HMap *, fptr);
static inline void HMap_free(HMap *hm) {
  const My_allocator *allocator = hm->links->allocator;
  List_free(hm->links);
  stringList_free(hm->KVs);
  aFree(allocator, hm->metadata);
  aFree(allocator, hm);
}

static inline void HMap_cleanup_handler(HMap **hm) {
  if (hm && *hm) {
    HMap_free(*hm);
    *hm = NULL;
  }
}
static inline void HMap_remake(HMap *hm) {
  stringList *l = stringList_remake(hm->KVs);
  stringList_free(hm->KVs);
  hm->KVs = l;
}
static inline stringList *HMap_getkeys(HMap *map) {
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
static inline size_t HMap_countCollisions(HMap *map) {
  return (map->links->length);
}

struct HMap_both {
  fptr key;
  fptr val;
};
struct HMap_both HMap_getBoth(HMap *map, fptr key);
#define HMap_scoped [[gnu::cleanup(HMap_cleanup_handler)]] HMap

#endif // HMAP_H

#ifdef HMAP_C
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
      mList_add(map->links, HMap_innertype, HMap_innerEmpty);
    } else {
      next_index = handle->next;
    }
    return HMap_setForce(
        map, (HMap_innertype *)List_getRef(map->links, next_index), key, val
    );
  }
}
unsigned int HMap_set(HMap *map, fptr key, fptr val) {
  unsigned int hash = HMap_hash(key);
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
#endif // HMAP_C
