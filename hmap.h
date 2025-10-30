#ifndef HMAP_H
#define HMAP_H
#include "stringList.h"
#include "um_fp.h"
#include "umap.h"
#include <stdint.h>
#include <stdlib.h>
#define HMap_MAXL (1000)
#define HMap_MAXHASH ((uint16_t)-1)
typedef struct {
  UMap_innertype kind;
  uint16_t index;
  uint16_t next;
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
      .kind = NORMAL, .index = HMap_MAXHASH, .next = HMap_MAXHASH})
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
unsigned int HMap_setForce(HMap *map, HMap_innertype *handle, um_fp key,
                           um_fp val) {
  if (handle->index != HMap_MAXHASH) {
    while (handle->next != HMap_MAXHASH &&
           um_fp_cmp(key, stringList_get(map->KVs, handle->index))) {
      handle = (HMap_innertype *)(stringList_get(map->KVs, handle->next).ptr);
    }
    if (handle->next == HMap_MAXHASH) {
      um_fp newKey = um_blockT(HMap_innertype, HMap_innerEmpty);
      handle->next = stringList_append(map->KVs, newKey);
      handle = (HMap_innertype *)(stringList_get(map->KVs, handle->next).ptr);
    }
  }
  handle->index = stringList_append(map->KVs, key);
  stringList_append(map->KVs, val);
  return handle->index;
}
unsigned int HMap_set(HMap *map, um_fp key, um_fp val) {
  unsigned int hash = HMap_hash(key);
  printf("hash of %.*s is %i, or %i\n",key.width,key.ptr,hash,hash%HMap_MAXL);
  HMap_innertype *ht = map->metadata + (hash % HMap_MAXL);
  return HMap_setForce(map, ht, key, val);
}
um_fp HMap_get(HMap *map, um_fp key) {
  HMap_innertype *handle = map->metadata + (HMap_hash(key) % HMap_MAXL);

  if (handle->index != HMap_MAXHASH) {
    while (handle->next != HMap_MAXHASH &&
           um_fp_cmp(key, stringList_get(map->KVs, handle->index))) {
      handle = (HMap_innertype *)(stringList_get(map->KVs, handle->next).ptr);
    }
    if (handle->next == HMap_MAXHASH) {
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
