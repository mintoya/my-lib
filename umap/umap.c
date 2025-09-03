#ifdef __cplusplus
extern "C" {
#endif
#include "umap.h"
#include "../alloc.h"
#include "../string-List/stringList.h"
#include <stdlib.h>
#include <string.h>

// typedef struct umap {
//   List *keys;   // chars
//   List *vals;   // chars
// } UMap;

unsigned int UMap_linearSearch(UMap *map, um_fp key) {
  unsigned int res = 0;
  unsigned int len = stringList_length( map->keys );

  while (res < len) {
    um_fp current = UMap_getKeyAtIndex(map, res);
    if (um_fp_cmp(key, current) <= 0) {
      return res;
    }
    res++;
  }
  return res;
}

UMap *UMap_new() {
  UMap *res = malloc(sizeof(UMap));
  *res = (UMap){
      .keys = stringList_new(),
      .vals = stringList_new(),
  };
  return res;
}

um_fp UMap_getKeyAtIndex(UMap *map, unsigned int index) {
  return stringList_get(map->keys, index);
};
um_fp UMap_getValAtKey(UMap *map, um_fp key) {
  return stringList_get(map->vals, UMap_linearSearch(map, key));
}

void UMap_set(UMap *map, um_fp key, um_fp val) {

  unsigned int index = UMap_linearSearch(map, key);
  unsigned int length = stringList_length(map->keys);

  if ( index< length && !um_fp_cmp(UMap_getKeyAtIndex(map, index), key)) {
    stringList_set(map->keys, key, index);
    stringList_set(map->vals, val, index);
  } else {
    stringList_insert(map->keys, key, index);
    stringList_insert(map->vals, val, index);
  }
};

// make a copy without unused keys & vals
// UMap *UMap_remake(UMap *map);

// void UMap_free(UMap *map);
/*
 * not modifiable
 * freeing keyReg itself deletes this
 * free(*((void**)&result)) ?
 */
// UMap UMap_fromBuf(um_fp mapRasterized);
// um_fp UMap_toBuf(UMap *map);

#ifdef __cplusplus
}
#endif
