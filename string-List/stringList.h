#ifndef STRING_LIST_H
#define STRING_LIST_H
#define initialBufferSize
#include "../my-list/my-list.h"
#include "um_fp.h"
#include <stdio.h>
#include <string.h>

typedef struct stringMetaData {
  unsigned int index;
  unsigned int width;
  unsigned int _size;
} stringMetaData;

typedef struct {
  List List_char;
  List List_stringMetaData;
} stringList;

stringList *stringList_new();
um_fp stringList_get(stringList *l, unsigned int index);
void stringList_insert(stringList *l, um_fp, unsigned int index);
void stringList_set(stringList *l, um_fp, unsigned int index);
void stringList_append(stringList *l, um_fp);
stringList *stringList_remake(stringList *);
void stringList_free(stringList *l);
static inline unsigned int stringList_length(stringList *l) {
  return l->List_stringMetaData.length;
}
// returns length if not found
unsigned int stringList_search(stringList *l, um_fp);

#endif // STRING_LIST_H

#ifdef STRING_LIST_C
#include <stdlib.h>
#include <string.h>

#ifndef STRING_LIST_minSize
#define STRING_LIST_minSize 20
#endif

stringList *stringList_new() {
  stringList *res = (stringList *)malloc(sizeof(stringList));
  // clang-format off
  *res = (stringList) {
    .List_char = {
      .width = sizeof(char),
      .length = 0,
      .size = 1,
      .head = (char*)malloc(sizeof(char)),
    },
    .List_stringMetaData = {
        .width = sizeof(stringMetaData),
        .length = 0,
        .size = 1,
        .head = (char*)malloc(sizeof(stringMetaData)),
    },

  };
  // clang-format on
  return res;
}
#define max(a, b) ((a < b) ? (b) : (a))

um_fp stringList_get(stringList *l, unsigned int index) {
  if (index > l->List_stringMetaData.length)
    return nullUmf;
  stringMetaData thisS =
      mList_get(&(l->List_stringMetaData), stringMetaData, index);
  return ((um_fp){.ptr = (List_getRef(&(l->List_char), thisS.index)),
                  .width = thisS.width});
}
stringList *stringList_remake(stringList *origional) {
  stringList *res = stringList_new();
  for (unsigned int i = 0; i < stringList_length(origional); i++) {
    stringList_append(res, stringList_get(origional, i));
  }
  return res;
}
void stringList_append(stringList *l, um_fp value) {
  unsigned int trueSize = max(value.width, STRING_LIST_minSize);
  stringMetaData thisS = {
      .index = l->List_char.length,
      .width = value.width,
      ._size = trueSize,
  };
  mList_add(&(l->List_stringMetaData), stringMetaData, thisS);
  List_appendFromArr(&(l->List_char), value.ptr, value.width);
  List_pad(&(l->List_char), trueSize - value.width);
}
void stringList_insert(stringList *l, um_fp value, unsigned int index) {
  unsigned int trueSize = max(value.width, STRING_LIST_minSize);
  stringMetaData thisS = {
      .index = l->List_char.length,
      .width = value.width,
      ._size = trueSize,
  };
  mList_insert(&(l->List_stringMetaData), stringMetaData, thisS, index);
  List_appendFromArr(&(l->List_char), value.ptr, value.width);
  List_pad(&(l->List_char), trueSize - value.width);
}
void stringList_set(stringList *l, um_fp value, unsigned int index) {
  stringMetaData thisS =
      mList_get(&(l->List_stringMetaData), stringMetaData, index);
  if (thisS._size < value.width) {
    stringMetaData *ref = List_getRef(&(l->List_stringMetaData), index);
    ref->width = value.width;
    ref->_size = value.width;
    ref->index = l->List_char.length;
    List_appendFromArr(&(l->List_char), value.ptr, value.width);
  } else {
    stringMetaData *ref = List_getRef(&(l->List_stringMetaData), index);
    ref->width = value.width;
    memcpy(List_getRef(&(l->List_char), ref->index), value.ptr, value.width);
  }
}

void stringList_free(stringList *l) {
  free(l->List_char.head);
  free(l->List_stringMetaData.head);
  free(l);
}
unsigned int stringList_search(stringList *l, um_fp what) {
  stringMetaData *meta = (stringMetaData *)l->List_stringMetaData.head;
  unsigned int res;
  unsigned int length = stringList_length(l);

  for (res = 0; res < length; res++) {
    if (meta[res].width == what.width &&
        !strncmp((char *)what.ptr,
                 (char *)List_getRef(&(l->List_char), meta[res].index),
                 what.width)) {
      return res;
    }
  }
  return res;
}
#undef max
#endif // STRING_LIST_C
