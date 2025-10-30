#ifndef HMAP_H
#define HMAP_H
#include "stringList.h"
#include "um_fp.h"
#include "umap.h"
#include <stdint.h>
#include <stdlib.h>
#define HMap_MAXHASH ((uint16_t)-1)
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
  List *links;
  stringList *KVs;
} HMap;
static inline size_t HMap_footprint(HMap *hm) {
  return stringList_footprint(hm->KVs) + HMap_MAXL * sizeof(HMap_innertype) +
         List_headArea(hm->links);
}

#define HMap_innerEmpty                                                        \
  ((HMap_innertype){                                                           \
      .kind = NORMAL, .index = 0, .next = 0, .hasindex = 0, .hasnext = 0})
unsigned int HMap_hash(um_fp); // unbounded

static inline HMap *HMap_new() {
  HMap *res = (HMap *)malloc(sizeof(HMap));
  *res = (HMap){.links = mList(HMap_innertype), .KVs = stringList_new()};
  for (int i = 0; i < HMap_MAXL; i++)
    res->metadata[i] = HMap_innerEmpty;
  return res;
}
unsigned int HMap_set(HMap *, um_fp key, um_fp val);
um_fp HMap_get(HMap *, um_fp);
um_fp HMap_get(HMap *map, um_fp key) {
  unsigned int hash = HMap_hash(key);
  HMap_innertype *ht = map->metadata + (hash % HMap_MAXL);
  while (1) {
    if (!um_fp_cmp(key, stringList_get(map->KVs, ht->index))) {
      return stringList_get(map->KVs, ht->index + 1);
    }
    if (!ht->hasnext)
      return nullUmf;
    ht = (HMap_innertype *)List_getRef(map->links, ht->next);
  }
}
static inline void HMap_free(HMap *hm) {
  List_free(hm->links);
  stringList_free(hm->KVs);
}

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
    handle->hasindex = 1;
    stringList_append(map->KVs, val);
    return handle->index;
  } else if (!um_fp_cmp(stringList_get(map->KVs, handle->index), key)) {
    stringList_set(map->KVs, val, handle->index + 1);
    return handle->index;
  } else {
    // collisoin
    if (!handle->hasnext) {
      handle->hasnext = 1;
      handle->next = map->links->length;
      mList_add(map->links, HMap_innertype, HMap_innerEmpty);
    }
    return HMap_setForce(
        map, (HMap_innertype *)List_getRef(map->links, handle->next), key, val);
  }
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
#endif // HMAP_C
