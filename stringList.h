#ifndef STRING_LIST_H
#define STRING_LIST_H
#include "allocator.h"
#include "my-list.h"
#include "um_fp.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  unsigned int index;
  size_t width;
  size_t _size;
} stringMetaData;

typedef struct {
  List List_char;
  List List_stringMetaData;
} stringList;
static inline size_t stringList_footprint(stringList *sl) {
  return List_fullHeadArea(&(sl->List_stringMetaData)) +
         List_fullHeadArea(&(sl->List_char));
}

typedef struct {
  fptr raw;
} stringListView;

stringList *stringList_new(const My_allocator *);
fptr stringList_get(stringList *l, unsigned int index);
ffptr stringList_getExt(stringList *l, unsigned int index);
void stringList_insert(stringList *l, fptr, unsigned int index);
void stringList_set(stringList *l, fptr, unsigned int index);
unsigned int stringList_append(stringList *l, fptr);

#define advance(dst, src, length)                                              \
  memcpy(dst, src, length);                                                    \
  dst += length / sizeof(uint8_t);
// memory layot:
//  { size_t:keycount | metadata buffer | data buffer }
static inline stringListView stringList_tobuf(stringList *l) {
  size_t area = List_headArea(&(l->List_stringMetaData)) +
                List_headArea(&(l->List_char)) + sizeof(size_t);
  size_t metalength = l->List_stringMetaData.length;
  fptr res = {
      .width = area,
      .ptr = (uint8_t *)aAlloc(l->List_char.allocator, area),
  };

  uint8_t *use = res.ptr;
  advance(use, &metalength, sizeof(size_t));
  advance(use, l->List_stringMetaData.head,
          List_headArea(&l->List_stringMetaData));
  advance(use, l->List_char.head, List_headArea(&l->List_char));

  return ((stringListView){.raw = res});
}

void stringListView_free(stringListView slv);
#undef advance
#define advance(dst, src, length)                                              \
  memcpy(dst, src, length);                                                    \
  src += length / sizeof(uint8_t);
static inline stringList *stringList_fromBuf(stringListView um) {
  uint8_t *ptr = um.raw.ptr;
  size_t metalength;
  advance(&metalength, ptr, sizeof(size_t));

  stringList *l = (stringList *)aAlloc((&defaultAllocator), sizeof(stringList));

  l->List_stringMetaData = (List){
      .width = sizeof(stringMetaData),
      .length = (unsigned int)metalength,
      .size = (unsigned int)metalength,
      .head = (uint8_t *)aAlloc((&defaultAllocator),
                                sizeof(stringMetaData) * metalength),
  };
  advance(l->List_stringMetaData.head, ptr,
          List_headArea(&(l->List_stringMetaData)));
  unsigned int charlength = (unsigned int)(um.raw.ptr + um.raw.width - ptr);
  l->List_char = (List){
      .width = sizeof(char),
      .length = charlength,
      .size = charlength,
      .head = (uint8_t *)aAlloc((&defaultAllocator), sizeof(char) * charlength),
  };

  advance(l->List_char.head, ptr, charlength);

  return l;
}
#undef advance

List stringListView_MetaList(stringListView slv);
List stringListView_CharList(stringListView slv);
fptr stringListView_get(stringListView slv, unsigned int index);

// returns length if not found
unsigned int stringList_search(stringList *l, fptr key);
// new sl with unused space removed
stringList *stringList_remake(stringList *);
void stringList_free(stringList *l);
static inline unsigned int stringList_length(stringList *l) {
  return l->List_stringMetaData.length;
}
static inline void stringList_cleanup_handler(stringList **sl) {
  if (sl && *sl)
    stringList_free(*sl);
  *sl = NULL;
}

#define stringList_scoped                                                      \
  stringList [[gnu::cleanup(stringList_cleanup_handler)]]

#endif // STRING_LIST_H

#ifdef STRING_LIST_C
#include <string.h>

#ifndef STRING_LIST_minSize
#define STRING_LIST_minSize 10
#endif

stringList *stringList_new(const My_allocator *allocator) {
  stringList *res = (stringList *)aAlloc(allocator, sizeof(stringList));
  // clang-format off
  *res = (stringList) {
    .List_char = {
      .width = sizeof(char),
      .length = 0,
      .size = 1,
      .head = (uint8_t*)aAlloc(allocator,sizeof(char)),
      .allocator = allocator,
    },
    .List_stringMetaData = {
      .width = sizeof(stringMetaData),
      .length = 0,
      .size = 1,
      .head = (uint8_t*)aAlloc(allocator,sizeof(stringMetaData)),
      .allocator = allocator,
    },

  };
  // clang-format on
  return res;
}
#define max(a, b) ((a < b) ? (b) : (a))

fptr stringList_get(stringList *l, unsigned int index) {
  if (index >= l->List_stringMetaData.length)
    return nullFptr;
  stringMetaData thisS =
      mList_get(&(l->List_stringMetaData), stringMetaData, index);
  return ((fptr){
      .width = thisS.width,
      .ptr = (uint8_t *)(List_getRef(&(l->List_char), thisS.index)),
  });
}
ffptr stringList_getExt(stringList *l, unsigned int index) {
  if (index >= l->List_stringMetaData.length)
    return nullFFptr;
  stringMetaData thisS =
      mList_get(&(l->List_stringMetaData), stringMetaData, index);
  return ((ffptr){
      .width = thisS.width,
      .ptr = (uint8_t *)(List_getRef(&(l->List_char), thisS.index)),
      .capacity = thisS._size,
  });
}
unsigned int stringList_search(stringList *l, fptr what) {
  stringMetaData *meta = (stringMetaData *)l->List_stringMetaData.head;
  unsigned int res = 0;
  unsigned int length = stringList_length(l);

  for (; res < length; res++) {
    fptr thisS = ((um_fp){
        .width = meta[res].width,
        .ptr = (uint8_t *)List_getRef(&(l->List_char), meta[res].index),
    });
    if (um_eq(thisS, what))
      return res;
  }
  return res;
}
stringList *stringList_remake(stringList *origional) {
  stringList *res = stringList_new(origional->List_char.allocator);
  for (unsigned int i = 0; i < stringList_length(origional); i++) {
    fptr item = stringList_get(origional, i);
    stringList_append(res, item);
  }
  return res;
}
unsigned int stringList_append(stringList *l, fptr value) {
  unsigned int trueSize = max(value.width, STRING_LIST_minSize);
  stringMetaData thisS = {
      .index = l->List_char.length,
      .width = value.width,
      ._size = trueSize,
  };
  mList_add(&(l->List_stringMetaData), stringMetaData, thisS);
  List_appendFromArr(&(l->List_char), value.ptr, value.width);
  List_pad(&(l->List_char), trueSize - value.width);
  return l->List_stringMetaData.length - 1;
}
void stringList_insert(stringList *l, fptr value, unsigned int index) {
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
void stringList_set(stringList *l, fptr value, unsigned int index) {
  stringMetaData thisS =
      mList_get(&(l->List_stringMetaData), stringMetaData, index);
  if (thisS._size < value.width) {
    stringMetaData *ref =
        (stringMetaData *)List_getRef(&(l->List_stringMetaData), index);
    ref->width = value.width;
    ref->_size = value.width;
    ref->index = l->List_char.length;
    List_appendFromArr(&(l->List_char), value.ptr, value.width);
  } else {
    stringMetaData *ref =
        (stringMetaData *)List_getRef(&(l->List_stringMetaData), index);
    ref->width = value.width;
    memcpy(List_getRef(&(l->List_char), ref->index), value.ptr, value.width);
  }
}

List stringListView_MetaList(stringListView slv) {
  size_t length = *(size_t *)slv.raw.ptr;
  List res = {
      .width = sizeof(stringMetaData),
      .length = (unsigned int)length,
      .size = (unsigned int)length,
      .head = slv.raw.ptr + sizeof(size_t),
  };
  return res;
}
List stringListView_CharList(stringListView slv) {
  size_t flength = *(size_t *)slv.raw.ptr;
  size_t length =
      slv.raw.width - (sizeof(size_t) + flength * sizeof(stringMetaData));
  List res = {
      .width = sizeof(char),
      .length = (unsigned int)length,
      .size = (unsigned int)length,
      .head = slv.raw.ptr + sizeof(size_t) + flength * sizeof(stringMetaData),
  };
  return res;
}
fptr stringListView_get(stringListView slv, unsigned int index) {
  stringList temp = (stringList){
      .List_char = stringListView_CharList(slv),
      .List_stringMetaData = stringListView_MetaList(slv),
  };
  return stringList_get(&temp, index);
}
void stringListView_free(stringListView slv) { free(slv.raw.ptr); }
void stringList_free(stringList *l) {
  const My_allocator *allocator = l->List_char.allocator;
  aFree(allocator, l->List_char.head);
  aFree(allocator, l->List_stringMetaData.head);
  aFree(allocator, l);
}

#endif // STRING_LIST_C
