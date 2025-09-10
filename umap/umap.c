#ifdef __cplusplus
extern "C" {
#endif
#include "umap.h"
#include "../string-List/stringList.h"
#include <stdlib.h>

// static unsigned int runs = 0;
unsigned int UMap_linearSearch(UMap *map, um_fp key) {
  unsigned int res = 0;
  unsigned int len = stringList_length(map->keys);
  while (res < len && um_fp_cmp(key, UMap_getKeyAtIndex(map, res)) > 0) {
    res++;
    // printf("linear search %i \n", runs++);
  }
  return res;
}
unsigned int UMap_binarySearch(UMap *map, um_fp key) {
  unsigned int region = stringList_length(map->keys);
  unsigned int front = 0;

  while (region > 2) {
    char less = um_fp_cmp(key, UMap_getKeyAtIndex(map, front + region / 2)) > 0;
    region /= 2;
    if (less)
      front += region;
    // printf("search %i \n", runs++);
  }

  unsigned int res = front;
  while (res < stringList_length(map->keys) &&
         um_fp_cmp(key, UMap_getKeyAtIndex(map, res)) > 0) {
    res++;
    // printf("linear search %i \n", runs++);
  }
  return res;
}
static inline unsigned int UMap_searchFunction(UMap *map, um_fp key) {
  return UMap_binarySearch(map, key);
}
UMap *UMap_new() {
  UMap *res = malloc(sizeof(UMap));
  *res = (UMap){
      .keys = stringList_new(),
      .vals = stringList_new(),
  };
  return res;
}

um_fp UMap_getValAtKey(UMap *map, um_fp key) {
  unsigned int index = UMap_searchFunction(map, key);

  if (!um_fp_cmp(key, UMap_getKeyAtIndex(map, index))) {
    return stringList_get(map->vals, index);
  }

  return nullUmf;
}

unsigned int UMap_set(UMap *map, um_fp key, um_fp val) {
  unsigned int index = UMap_searchFunction(map, key);
  unsigned int length = stringList_length(map->keys);

  if (index < length && !um_fp_cmp(UMap_getKeyAtIndex(map, index), key)) {
    stringList_set(map->keys, key, index);
    stringList_set(map->vals, val, index);
  } else {
    stringList_insert(map->keys, key, index);
    stringList_insert(map->vals, val, index);
  }
  return index;
};

void UMap_addChild(UMap *map, um_fp key, UMap *ref) {
  UMap_set(map, key, (um_fp){.ptr = &ref, .width = sizeof(UMap *)});
}
void UMap_free(UMap *map){
  stringList_free(map->keys);
  stringList_free(map->vals);
  free(map);
}

#ifdef __cplusplus
}
#endif
