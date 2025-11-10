#ifndef HMAP_H
#define HMAP_H
#include "stringList.h"
#include "um_fp.h"
#include <stdint.h>
#include <stdlib.h>
typedef struct {
  uint16_t index;
  uint16_t next;
  unsigned char hasindex : 1;
  unsigned char hasnext : 1;
} HMap_innertype;
typedef struct {
  size_t metaSize;
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

#define HMap_innerEmpty                                                        \
  ((HMap_innertype){.index = 0, .next = 0, .hasindex = 0, .hasnext = 0})
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// #TODO maybe add this submodule
// faster i think
// #include "komihash/komihash.h"
// __attribute__((pure)) static inline unsigned int HMap_hash(um_fp str) {
//   return komihash(str.ptr, str.width, 6767);
// }
static const size_t HMap_h = (0x67676141420);
__attribute__((pure)) static inline unsigned int HMap_hash(um_fp str) {
  size_t h = HMap_h;
  size_t arrLength = (str.width) / sizeof(uintptr_t);
  size_t rest = str.width % sizeof(uintptr_t);
  for (size_t i = 0; i < arrLength; i++) {
    uintptr_t t;
    memcpy(&t, str.ptr + (i * sizeof(uintptr_t)), sizeof(uintptr_t));
    h = (h << 2) + h ^ t;
  }
  for (size_t i = str.width - rest; i < str.width; i++)
    h = (h << 2) + h ^ (str.ptr)[i];
  return h;
}

static inline HMap *HMap_new(size_t metaSize) {

  HMap *res = (HMap *)malloc(sizeof(HMap));
  // clang-format off
  *res = (HMap){
    .metaSize = metaSize,
    .metadata = (HMap_innertype*)malloc(metaSize*sizeof(HMap_innertype)),
    .links = mList(HMap_innertype), 
    .KVs = stringList_new(),
  };
  List_resize(res->links, metaSize/10);
  // clang-format on
  memset(res->metadata, 0, sizeof(HMap_innertype) * metaSize);
  return res;
}
unsigned int HMap_set(HMap *, um_fp key, um_fp val);
um_fp HMap_get(HMap *, um_fp);
static inline void HMap_free(HMap *hm) {
  List_free(hm->links);
  stringList_free(hm->KVs);
  free(hm->metadata);
  free(hm);
}

static inline void HMap_cleanup_handler(HMap **hm) {
  if (hm && *hm)
    HMap_free(*hm);
  *hm = NULL;
}
static inline void HMap_remake(HMap *hm) {
  stringList *l = stringList_remake(hm->KVs);
  stringList_free(hm->KVs);
  hm->KVs = l;
}
static inline stringList *HMap_getkeys(HMap *map) {
  stringList *keys = stringList_new();

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
  um_fp key;
  um_fp val;
};
struct HMap_both HMap_getBoth(HMap *map, um_fp key);
#define HMap_scoped HMap __attribute__((cleanup(HMap_cleanup_handler)))

#endif // HMAP_H

// #define HMAP_C
#ifdef HMAP_C
unsigned int HMap_setForce(HMap *map, HMap_innertype *handle, um_fp key,
                           um_fp val) {
  if (!handle->hasindex) {
    handle->index = stringList_append(map->KVs, key);
    handle->hasindex = 1;
    stringList_append(map->KVs, val);
    return handle->index;
  } else if (!um_fp_cmp(stringList_get(map->KVs, handle->index), key)) {
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
        map, (HMap_innertype *)List_getRef(map->links, next_index), key, val);
  }
}
unsigned int HMap_set(HMap *map, um_fp key, um_fp val) {
  unsigned int hash = HMap_hash(key);
  HMap_innertype *ht = map->metadata + (hash % map->metaSize);
  return HMap_setForce(map, ht, key, val);
}
um_fp HMap_get(HMap *map, um_fp key) {
  unsigned int hash = HMap_hash(key);
  HMap_innertype *ht = map->metadata + (hash % map->metaSize);
  while (1) {
    if (!um_fp_cmp(key, stringList_get(map->KVs, ht->index))) {
      return stringList_get(map->KVs, ht->index + 1);
    }
    if (!ht->hasnext)
      return nullUmf;
    ht = (HMap_innertype *)List_getRef(map->links, ht->next);
  }
}
struct HMap_both HMap_getBoth(HMap *map, um_fp key) {
  unsigned int hash = HMap_hash(key);
  HMap_innertype *ht = map->metadata + (hash % map->metaSize);
  while (1) {
    if (!um_fp_cmp(key, stringList_get(map->KVs, ht->index))) {
      return (struct HMap_both){
          .key = stringList_get(map->KVs, ht->index),
          .val = stringList_get(map->KVs, ht->index + 1),
      };
    }
    if (!ht->hasnext)
      return (struct HMap_both){nullUmf, nullUmf};
    ht = (HMap_innertype *)List_getRef(map->links, ht->next);
  }
}
#endif // HMAP_C
