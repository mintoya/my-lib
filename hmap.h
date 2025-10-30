#ifndef HMAP_H
#define HMAP_H
#include "stringList.h"
#include "um_fp.h"
#include "umap.h"
#include <stdint.h>
#include <stdlib.h>
#define HMap_MAXHASH ((uint16_t)2)
#define HMap_MAXL (HMap_MAXHASH - 1)
typedef struct {
  UMap_innertype kind;
  uint16_t index;
  uint16_t next;
  unsigned char hasindex : 1;
  unsigned char hasnext : 1;
} HMap_innertype;
typedef struct {
  HMap_innertype metadata[HMap_MAXL];
  stringList *KVs;
} HMap;
static inline size_t HMap_footprint(HMap *hm) {
  return stringList_footprint(hm->KVs) + HMap_MAXL * sizeof(HMap_innertype);
}

#define HMap_innerEmpty                                                        \
  ((HMap_innertype){                                                           \
      .kind = NORMAL, .index = 0, .next = 0, .hasindex = 0, .hasnext = 0})
unsigned int HMap_hash(um_fp); // unbounded

static inline HMap *HMap_new() {
  HMap *res = (HMap *)malloc(sizeof(HMap));
  *res = (HMap){.KVs = stringList_new()};
  for (int i = 0; i < HMap_MAXL; i++)
    res->metadata[i] = HMap_innerEmpty;
  return res;
}
unsigned int HMap_set(HMap *, um_fp key, um_fp val);
um_fp HMap_get(HMap *, um_fp);
static inline void HMap_free(HMap *hm) { stringList_free(hm->KVs); }

static inline void HMap_cleanup_handler(HMap **hm) {
  if (hm && *hm)
    HMap_free(*hm);
  *hm = NULL;
}
static inline void HMap_remake(HMap *hm) {
  stringList *l;
  l = stringList_remake(hm->KVs);
  stringList_free(hm->KVs);
  hm->KVs = l;
}
unsigned int HMap_setForce(HMap *map, HMap_innertype *handle, um_fp key,
                           um_fp val) {
  if (!handle->hasindex) {
    handle->index = stringList_append(map->KVs, key);
    stringList_append(map->KVs, val);
    handle->hasindex = 1;
  } else if (!um_fp_cmp(key, stringList_get(map->KVs, handle->index))) {
    stringList_set(map->KVs, val, handle->index + 1);
  } else {
    while (handle->hasnext) {
      handle = (HMap_innertype *)(stringList_get(map->KVs, handle->next).ptr);
      if (!um_fp_cmp(key, stringList_get(map->KVs, handle->index))) {
        return HMap_setForce(map, handle, key, val);
      }
    }
    HMap_innertype newHandle = (HMap_innertype){
        .index = (uint16_t)stringList_append(map->KVs, key),
        .next = 0,
        .hasindex = 1,
        .hasnext = 0,
    };
    stringList_append(map->KVs, val);
    handle->next =
        stringList_append(map->KVs, ((um_fp){.ptr = (uint8_t *)&(newHandle),
                                             .width = sizeof(HMap_innertype)}));
    handle->hasnext = 1;
  }
  return handle->index;
}
#define HMap_scoped HMap __attribute__((cleanup(HMap_cleanup_handler)))

#endif // HMAP_H

// #define HMAP_C
#ifdef HMAP_C
unsigned int HMap_hash(um_fp str) {
  unsigned long h = 5381;
  for (; str.width > 0; str.width--)
    h = ((h << 5) + h) + (str.ptr)[str.width - 1];
  return h;
}
unsigned int HMap_set(HMap *map, um_fp key, um_fp val) {
  unsigned int hash = HMap_hash(key);
  HMap_innertype *ht = map->metadata + (hash % HMap_MAXL);
  return HMap_setForce(map, ht, key, val);
}
um_fp HMap_get(HMap *map, um_fp key) {
  HMap_innertype *handle = map->metadata + (HMap_hash(key) % HMap_MAXL);

  if (handle->hasindex) {
    while (handle->hasnext &&
           um_fp_cmp(key, stringList_get(map->KVs, handle->index))) {
      handle = (HMap_innertype *)(stringList_get(map->KVs, handle->next).ptr);
    }
    if (!handle->hasnext) {
      um_fp foundKey = stringList_get(map->KVs, handle->index);
      return (um_fp_cmp(foundKey, key))
                 ? (nullUmf)
                 : (stringList_get(map->KVs, handle->index + 1));
    } else {
      return stringList_get(map->KVs, handle->index + 1);
    }
  } else {
    um_fp foundKey = stringList_get(map->KVs, handle->index);
    return (um_fp_cmp(foundKey, key))
               ? (nullUmf)
               : (stringList_get(map->KVs, handle->index + 1));
  }
}
#endif // HMAP_C
