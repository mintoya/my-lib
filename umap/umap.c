#ifdef __cplusplus
extern "C" {
#endif
#include "../string-List/stringList.h"
#include <stdlib.h>
#include <string.h>
#include "umap.h"
#include "../alloc.h"
#include "../string-List/stringList.h"

// typedef struct umap {
//   List *keys;   // chars
//   List *vals;   // chars
// } UMap;

static inline int keyCmp(um_fp a, um_fp b) {
  int res = (strncmp(a.ptr, b.ptr, (a.width < b.width) ? a.width : b.width));
  return (res)?(res):( a.width-b.width);
}

unsigned int UMap_linearSearch(UMap *map, um_fp key) {
  unsigned int res = 0;
  um_fp current = UMap_getKeyAtIndex(map, res);
  while (res < map->keys->List_stringMetaData.length && keyCmp(key, current) < 0 ) {
    res++;
    current = UMap_getKeyAtIndex(map, res);
  }
  return res;
}

UMap *UMap_new(){
  UMap* res = malloc(sizeof(UMap));
  *res = (UMap){
    .keys = stringList_new(),
    .vals = stringList_new(),
  };
  return res;
}

um_fp UMap_getKeyAtIndex(UMap *map, unsigned int index){
  return stringList_get(map->keys,index);
};
um_fp UMap_getValAtKey(UMap *map, um_fp key){
  return stringList_get(map->keys,UMap_linearSearch(map,key));
}

void UMap_set(UMap *map, um_fp key, um_fp val){
  unsigned int index = UMap_linearSearch(map,key);
  if(!keyCmp(UMap_getKeyAtIndex(map,index),key)){
    stringList_set(map->keys,key,index);
    stringList_set(map->vals,val,index);
  }
  else{
    stringList_insert(map->keys,key,index);
    stringList_insert(map->vals,val,index);
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
