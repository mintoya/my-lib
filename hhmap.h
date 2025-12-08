#if !defined(HHMAP_H)
  #define HHMAP_H (1)
// same as hmap, but key and value size are known
// still returns an fptr
typedef struct HHMap HHMap;
  #include "hmap.h"
  #include "my-list.h"

// exports
HHMap *HHMap_new(usize kSize, usize vSize, const My_allocator *allocator, u32 metaSize);
// crops or expands (with 0's) the keys and vals
// slower if metasize isnt 0
// if any argument is 0, it will assume you want the same one of that on the last ptr
void HHMap_transform(HHMap **last, usize kSize, usize vSize, const My_allocator *allocator, u32 metaSize);
void HHMap_free(HHMap *hm);
// void HHMap_remake(HMap *hm);
void *HHMap_get(HHMap *, void *);
u32 HHMap_set(HHMap *map, void *key, void *val);
u32 HHMap_getMetaSize(HHMap *);
u32 HHMap_count(const HHMap *map);
extern inline void *HHMap_getKey(const HHMap *map, u32 n); // linked to getKey, but otherwise meaningless
extern inline void *HHMap_getVal(const HHMap *map, u32 n); // linked to getVal, but otherwise meaningless

static inline void HHMap_cleanup_handler(HHMap **v) {
  if (v && *v) {
    HHMap_free(*v);
    *v = NULL;
  }
}
  #define HHMap_scoped [[gnu::cleanup(HHMap_cleanup_handler)]] HHMap

#endif // HHMAP_H

#if (defined(__INCLUDE_LEVEL__) && __INCLUDE_LEVEL__ == 0)
  #define HHMAP_C (1)
#endif
#ifdef HHMAP_C
  #include "fptr.h"
typedef struct {
  u32 index;
  u32 next;
  bool hasnext : 1;
  bool hasindex : 1;
} HHMap_innertype;
typedef struct HHMap {
  List KVs;   // List<k+v>
  List links; // List<hhmap_innertype>
  usize metaSize;
  // valsize = KVs.width-keysize
  usize keysize;
  HHMap_innertype metadata[];
} HHMap;

umax HHMap_hash(const fptr str) {
  umax hash = 5381;
  usize s = 0;
  for (; s < str.width; s++)
    hash = ((hash << 5) + hash) + (str.ptr[s]);
  return hash;
}
u32 HHMap_getMetaSize(HHMap *hm) {
  return hm->metaSize;
}
u32 HHMap_count(const HHMap *hm) {
  return hm->KVs.length;
}
HHMap *HHMap_new(usize kSize, usize vSize, const My_allocator *allocator, u32 metaSize) {
  HHMap *res = (HHMap *)aAlloc(
      allocator,
      sizeof(HHMap) + metaSize * sizeof(HHMap_innertype)
  );
  *res = (HHMap){
      .KVs = (List){
          kSize + vSize,
          0,
          5,
          (u8 *)aAlloc(allocator, (kSize + vSize) * 5),
          allocator,
      },
      .links = (List){
          sizeof(HHMap_innertype),
          0,
          5,
          (u8 *)aAlloc(allocator, sizeof(HHMap_innertype) * 5),
          allocator,
      },
      .metaSize = metaSize,
      .keysize = kSize,
  };
  lmemset(
      res->metadata,
      metaSize,
      ((HHMap_innertype){0})
  );
  return res;
}

void HHMap_transform(HHMap **last, usize kSize, usize vSize, const My_allocator *allocator, u32 metaSize) {
  assertMessage(last && *last);
  HHMap *lastmap = *last;
  if (!allocator)
    allocator = lastmap->KVs.allocator;
  if (!kSize)
    kSize = lastmap->keysize;
  if (!vSize)
    vSize = lastmap->KVs.width - kSize;
  bool sameMeta = !metaSize;
  if (sameMeta)
    metaSize = lastmap->metaSize;
  assertMessage(kSize && vSize && allocator);
  HHMap *res = HHMap_new(kSize, vSize, allocator, metaSize);
  if (sameMeta) {
  } else {
    for (u32 i = 0; i < HHMap_count(lastmap); i++) {
      // HHMap_set(res,HH);
    }
  }
}
void HHMap_free(HHMap *hm) {
  const My_allocator *allocator = hm->KVs.allocator;
  aFree(allocator, hm->links.head);
  aFree(allocator, hm->KVs.head);
  aFree(allocator, hm);
}
inline void *HHMap_getKey(const HHMap *map, u32 n) {
  assertMessage(map->keysize < map->KVs.width);
  usize keysize = map->keysize;
  u8 *k = (u8 *)List_getRef(&(map->KVs), n);
  return k;
}
inline void *HHMap_getVal(const HHMap *map, u32 n) {
  assertMessage(map->keysize < map->KVs.width);
  usize keysize = map->keysize;
  u8 *k = (u8 *)List_getRef(&(map->KVs), n);
  return (k + keysize);
}
u32 HHMap_setForce(HHMap *map, HHMap_innertype *handle, void *key, void *val, bool createKey) {
  assertMessage(map->metaSize != 0, "map.metaSize cant be 0");
  usize keysize = map->keysize;
  usize valsize = map->KVs.width - map->keysize;
  while (1) {
    if (handle->hasindex && !memcmp(List_getRef(&(map->KVs), handle->index), key, keysize)) {
      // handle exists && is the correct
      if (createKey) {
        u8 *valptr = (u8 *)List_getRef(&(map->KVs), handle->index) + keysize;
        if (val) {
          memcpy(valptr, val, valsize);
          handle->hasindex = true;
        } else {
          memset(valptr, 0, valsize);
          handle->hasindex = false;
        }
      }
      return handle->index;
    } else if (!handle->hasindex) {
      if (createKey) {
        handle->index = List_length(&(map->KVs));
        List_append(&(map->KVs), NULL);
        u8 *handleMem = (u8 *)List_getRef(&(map->KVs), handle->index);
        memcpy(handleMem, key, keysize);
        if (val) {
          memcpy(handleMem + keysize, val, valsize);
          handle->hasindex = true;
        } else {
          memset(handleMem + keysize, 0, valsize);
          handle->hasindex = false;
        }
        return handle->index;
      } else {
        if (!handle->hasnext)
          return map->KVs.length;
      }
    }
    if (!handle->hasnext) {
      if (createKey) {
        handle->next = map->links.length;
        handle->hasnext = true;
        List_append(&(map->links), NULL);
      } else {
        return map->KVs.length;
      }
    }
    handle = (HHMap_innertype *)List_getRef(&(map->links), handle->next);
  }
}

u32 HHMap_set(HHMap *map, void *key, void *val) {
  unsigned int hash = HHMap_hash((fptr){map->keysize, (u8 *)key});
  HHMap_innertype *ht = map->metadata + (hash % map->metaSize);
  return HHMap_setForce(map, ht, key, val, true);
}

void *HHMap_get(HHMap *map, void *key) {
  usize keysize = map->keysize;
  usize valsize = map->KVs.width - map->keysize;
  unsigned int hash = HHMap_hash((fptr){map->keysize, (u8 *)key});
  HHMap_innertype *ht = map->metadata + (hash % map->metaSize);
  u32 listLocatoin = HHMap_setForce(map, ht, key, NULL, false);
  if (listLocatoin == map->KVs.length)
    return NULL;
  u8 *element = (u8 *)List_getRef(&(map->KVs), listLocatoin) + keysize;
  return element;
}
#endif // HHMAP_C
