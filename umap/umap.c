#ifdef __cplusplus
extern "C" {
#endif
#include "umap.h"
#include "../string-List/stringList.h"
#include <stdlib.h>

unsigned int UMap_linearSearch(UMap *map, um_fp key) {
  unsigned int res = 0;
  unsigned int len = stringList_length(map->keys);
  while (res < len && um_fp_cmp(key, UMap_getKeyAtIndex(map, res)) > 0) {
    res++;
  }
  return res;
}
enum {
  RAW = 0,
  MAP = 1,
} meta;
UMap *UMap_new() {
  UMap *res = malloc(sizeof(UMap));
  *res = (UMap){
      .keys = stringList_new(),
      .vals = stringList_new(),
      .refs = mList(meta),
  };
  return res;
}

um_fp UMap_getKeyAtIndex(UMap *map, unsigned int index) {
  return stringList_get(map->keys, index);
};
um_fp UMap_getValAtIndex(UMap *map, unsigned int index) {
  return stringList_get(map->vals, index);
};

um_fp UMap_getValAtKey(UMap *map, um_fp key) {
  unsigned int index = UMap_linearSearch(map, key);

  if (!um_fp_cmp(key, UMap_getKeyAtIndex(map, index))) {
    return stringList_get(map->vals, index);
  }

  return nullUmf;
}

unsigned int  UMap_set(UMap *map, um_fp key, um_fp val) {
  unsigned int index = UMap_linearSearch(map, key);
  unsigned int length = stringList_length(map->keys);

  if (index < length && !um_fp_cmp(UMap_getKeyAtIndex(map, index), key)) {
    stringList_set(map->keys, key, index);
    stringList_set(map->vals, val, index);
  } else {
    stringList_insert(map->keys, key, index);
    stringList_insert(map->vals, val, index);
  }
};

void UMap_addChild(UMap *map, um_fp key, UMap *ref) {
  UMap_set(map, key, (um_fp){.ptr = &ref, .width = sizeof(UMap *)});

}

#ifdef __cplusplus
}
#endif
