#ifndef HMAP_H
#define HMAP_H
#include "stringList.h"
#include "um_fp.h"
#include "umap.h"
#include <cstdint>
#include <cstring>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#define HMap_MAXHASH ((uint16_t)2048)
typedef struct {
  UMap_innertype kind;
  uint16_t index;
  uint16_t next;
} HMap_innertype;
typedef struct {
  HMap_innertype metadata[HMap_MAXHASH];
  stringList *KVs;
} HMap;

#define HMap_innerEmpty                                                        \
  ((HMap_innertype){                                                           \
      .kind = NORMAL, .index = HMap_MAXHASH, .next = HMap_MAXHASH})
unsigned int HMap_hash(um_fp); // unbounded

static inline HMap *HMap_new() {
  HMap *res = (HMap *)malloc(sizeof(HMap));
  *res = {.KVs = stringList_new()};
  for (int i = 0; i < HMap_MAXHASH; i++)
    res->metadata[i] = HMap_innerEmpty;
  return res;
}
uint16_t HMap_set(HMap *, um_fp key, um_fp val);
um_fp HMap_get(HMap *, um_fp);

#endif // HMAP_H

#ifdef HMAP_C
unsigned int HMap_hash(um_fp str) {
  unsigned long h = 5381;
  for (; str.width > 0; str.width--)
    h = ((h << 5) + h) + (str.ptr)[str.width - 1];
  return h;
}
uint16_t HMap_setForce(HMap *map, HMap_innertype *handle, um_fp key,
                       um_fp val) {
  if (handle->index != HMap_MAXHASH) {
    while (handle->next != HMap_MAXHASH &&
           um_fp_cmp(key, stringList_get(map->KVs, handle->index)) != 0) {
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
uint16_t HMap_set(HMap *map, um_fp key, um_fp val) {
  HMap_innertype *ht = map->metadata + (HMap_hash(key) % HMap_MAXHASH);
  return HMap_setForce(map, ht, key, val);
}
#endif // HMAP_C
