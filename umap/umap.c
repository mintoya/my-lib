#include <stdlib.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "umap.h"
#include "../alloc.h"

struct _umapListElement {
  unsigned int index; // position of string in Map reg
  unsigned int width;
};
UMap *UMap_new() {
  UMap *res = regularAllocate(sizeof(UMap));
  *res = (UMap){.keyReg = mList(struct _umapListElement),
                .keys = mList(char),
                .valReg = mList(struct _umapListElement),
                .vals = mList(char)};
  return res;
}

um_fp UMap_getKeyAtIndex(UMap *map, unsigned int index) {
  struct _umapListElement *el = List_gst(map->keyReg, index);
  return (um_fp){.length = el->width,
                 .ptr = map->keys->head + el->index * sizeof(char)};
}
um_fp UMap_getValAtIndex(UMap *map, unsigned int index) {
  struct _umapListElement *el = List_gst(map->valReg, index);
  return (um_fp){.length = el->width,
                 .ptr = map->vals->head + el->index * sizeof(char)};
}
int keyCmp(um_fp a, um_fp b) {
  int res =
      (strncmp(a.ptr, b.ptr, (a.length < b.length) ? a.length : b.length));
  if (res)
    return res;
  return a.length - b.length;
}
int UMap_binarySearch(UMap *map, um_fp key) {
  int len = map->keyReg->length;
  if (len == 0) {
    return 0;
  }

  int left = 0;
  int right = len;

  while (left < right) {
    int mid = left + (right - left) / 2;
    um_fp midKey = UMap_getKeyAtIndex(map, mid);
    if (keyCmp(key, midKey) < 0) {
      left = mid + 1; // Search later part
    } else {
      right = mid; // Search earlier part
    }
  }

  return left;
}

int UMap_linearSearch(UMap *map, um_fp key) {
  if (map->keyReg->length == 0) {
    return 0;
  }
  int res = 0;
  um_fp current = UMap_getKeyAtIndex(map, res);
  while (keyCmp(key, current) < 0 && res < map->keyReg->length) {
    res++;
    current = UMap_getKeyAtIndex(map, res);
  }
  return res;
}

um_fp UMap_getValAtKey(UMap *map, um_fp key) {
  unsigned int index = UMap_binarySearch(map, key);
  if (!keyCmp(key, UMap_getKeyAtIndex(map, index))) {
    return UMap_getValAtIndex(map, UMap_binarySearch(map, key));
  } else {
    return nullUmf;
  }
}
void UMap_set(UMap *map, um_fp key, um_fp val) {
  unsigned int index = UMap_binarySearch(map, key);
  if (!keyCmp(key, UMap_getKeyAtIndex(map, index))) {
    *(struct _umapListElement *)List_gst(map->valReg, index) =
        (struct _umapListElement){.index = map->vals->length,
                                  .width = val.length};
    List_appendFromArr(map->vals, val.ptr, val.length);
  } else {
    struct _umapListElement keyElement = {.index = map->keys->length,
                                          .width = key.length};
    struct _umapListElement valElement = {.index = map->vals->length,
                                          .width = val.length};
    List_appendFromArr(map->keys, key.ptr, key.length);
    List_appendFromArr(map->vals, val.ptr, val.length);
    List_insert(map->keyReg, index, &keyElement);
    List_insert(map->valReg, index, &valElement);
  }
}

um_fp UMap_get(UMap *map, char *key) {
  return UMap_getValAtKey(map, um_from(key));
}

UMap* UMap_remake( UMap *map) {
  UMap *res = UMap_new();
  for (unsigned int i = 0; i < (map)->keyReg->length; i++) {
    um_fp key = UMap_getKeyAtIndex(map, i);
    um_fp val = UMap_getValAtIndex(map, i);
    if (val.length > 0)
      UMap_set(res, key, val);
  }
  return res;
}
void UMap_free(UMap *map) {
  List_free((map)->keyReg);
  List_free((map)->keys);
  List_free((map)->valReg);
  List_free((map)->vals);
  free(map);
}

um_fp UMap_toBuf(UMap* u){
  // UMap_remake(u);
  um_fp k = {
    .ptr = List_toBuffer(u->keys),
    .length = sizeof(List)+u->keys->length*u->keys->width
  };
  um_fp kr = {
    .ptr = List_toBuffer(u->keyReg),
    .length = sizeof(List)+u->keyReg->length*u->keyReg->width
  };
  um_fp v = {
    .ptr = List_toBuffer(u->vals),
    .length = sizeof(List)+u->vals->length*u->vals->width
  };
  um_fp vr = {
    .ptr = List_toBuffer(u->valReg),
    .length = sizeof(List)+u->valReg->length*u->valReg->width
  };
  unsigned int totalLength = k.length+kr.length+v.length+vr.length;
  void* result = regularAllocate(totalLength);
  memcpy(result , kr.ptr, kr.length);
  memcpy(result +kr.length , k.ptr, k.length);
  memcpy(result +kr.length +k.length , vr.ptr, vr.length);
  memcpy(result +kr.length +k.length +vr.length, v.ptr, v.length);
 
  freAllocate(k.ptr);
  freAllocate(kr.ptr);
  freAllocate(v.ptr);
  freAllocate(vr.ptr);

  return (um_fp){
    .ptr = result,
    .length = totalLength,
  };
}
UMap UMap_fromBuf(um_fp mRast){
  List* kr = List_fromBuffer(mRast.ptr);
  mRast.ptr+=sizeof(List) + kr->length*kr->width;
  List* k = List_fromBuffer(mRast.ptr);
  mRast.ptr+=sizeof(List) + k->length*k->width;
  List* vr = List_fromBuffer(mRast.ptr);
  mRast.ptr+=sizeof(List) + vr->length*vr->width;
  List* v = List_fromBuffer(mRast.ptr);
  return (UMap)  {
    .keyReg = kr,
    .keys = k,
    .valReg = vr,
    .vals = v
  };

}
#ifdef __cplusplus
}
#endif
