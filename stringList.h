#ifndef STRING_LIST_H
#define STRING_LIST_H
#include "my-list.h"
#include "um_fp.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  um_fp n;
  size_t nl;
} um_fp_extended;
#define nullUmfExt ((um_fp_extended){.n = nullUmf,.nl = 0})
typedef struct stringMetaData {
  unsigned int index;
  size_t width;
  size_t _size;
} stringMetaData;

typedef struct {
  List List_char;
  List List_stringMetaData;
} stringList;
static inline size_t stringList_footprint(stringList *sl) {
  return List_headArea(&(sl->List_stringMetaData)) +
         List_headArea(&(sl->List_char));
}

typedef struct {
  um_fp raw;
} stringListView;

stringList *stringList_new();
// returns null if over limit
um_fp stringList_get(stringList *l, unsigned int index);
um_fp_extended stringList_getExt(stringList *l, unsigned int index);
void stringList_insert(stringList *l, um_fp, unsigned int index);
void stringList_set(stringList *l, um_fp, unsigned int index);
unsigned int stringList_append(stringList *l, um_fp);

#define advance(dst, src, length)                                              \
  memcpy(dst, src, length);                                                    \
  dst += length / sizeof(uint8_t);
// memory layot:
//  { size_t:keycount | metadata buffer | data buffer }
static inline stringListView stringList_tobuf(stringList *l) {
  size_t area = List_headArea(&(l->List_stringMetaData)) +
                List_headArea(&(l->List_char)) + sizeof(size_t);
  size_t metalength = l->List_stringMetaData.length;
  um_fp res = {.ptr = (uint8_t *)malloc(area), .width = area};

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

  stringList *l = (stringList *)malloc(sizeof(stringList));

  l->List_stringMetaData = (List){
      .width = sizeof(stringMetaData),
      .length = (unsigned int)metalength,
      .size = (unsigned int)metalength,
      .head = (uint8_t *)malloc(sizeof(stringMetaData) * metalength),
  };
  advance(l->List_stringMetaData.head, ptr,
          List_headArea(&(l->List_stringMetaData)));
  unsigned int charlength = (unsigned int)(um.raw.ptr + um.raw.width - ptr);
  l->List_char = (List){.width = sizeof(char),
                        .length = charlength,
                        .size = charlength,
                        .head = (uint8_t *)malloc(sizeof(char) * charlength)};
  advance(l->List_char.head, ptr, charlength);

  return l;
}
#undef advance

List stringListView_MetaList(stringListView slv);
List stringListView_CharList(stringListView slv);
um_fp stringListView_get(stringListView slv, unsigned int index);

// returns length if not found
unsigned int stringList_search(stringList *l, um_fp key);
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
  stringList __attribute__((cleanup(stringList_cleanup_handler)))

#endif // STRING_LIST_H

#ifdef STRING_LIST_C
#include <stdlib.h>
#include <string.h>

#ifndef STRING_LIST_minSize
#define STRING_LIST_minSize 10
#endif

stringList *stringList_new() {
  stringList *res = (stringList *)malloc(sizeof(stringList));
  // clang-format off
  *res = (stringList) {
    .List_char = {
      .width = sizeof(char),
      .length = 0,
      .size = 1,
      .head = (uint8_t*)malloc(sizeof(char)),
    },
    .List_stringMetaData = {
      .width = sizeof(stringMetaData),
      .length = 0,
      .size = 1,
      .head = (uint8_t*)malloc(sizeof(stringMetaData)),
    },

  };
  // clang-format on
  return res;
}
#define max(a, b) ((a < b) ? (b) : (a))

um_fp stringList_get(stringList *l, unsigned int index) {
  if (index >= l->List_stringMetaData.length)
    return nullUmf;
  stringMetaData thisS =
      mList_get(&(l->List_stringMetaData), stringMetaData, index);
  return ((um_fp){.ptr = (uint8_t *)(List_getRef(&(l->List_char), thisS.index)),
                  .width = thisS.width});
}
um_fp_extended stringList_getExt(stringList *l, unsigned int index) {
  if (index >= l->List_stringMetaData.length)
    return nullUmfExt;
  stringMetaData thisS =
      mList_get(&(l->List_stringMetaData), stringMetaData, index);
  um_fp res =
      ((um_fp){.ptr = (uint8_t *)(List_getRef(&(l->List_char), thisS.index)),
               .width = thisS.width});
  return ((um_fp_extended){.n = res, .nl = thisS._size});
}
unsigned int stringList_search(stringList *l, um_fp what) {
  stringMetaData *meta = (stringMetaData *)l->List_stringMetaData.head;
  unsigned int res = 0;
  unsigned int length = stringList_length(l);

  for (res; res < length; res++) {
    um_fp thisS = ((um_fp){
        .ptr = (uint8_t *)List_getRef(&(l->List_char), meta[res].index),
        .width = meta[res].width});
    if (um_eq(thisS, what))
      return res;
  }
  return res;
}
stringList *stringList_remake(stringList *origional) {
  stringList *res = stringList_new();
  for (unsigned int i = 0; i < stringList_length(origional); i++) {
    um_fp item = stringList_get(origional, i);
    stringList_append(res, item);
  }
  return res;
}
unsigned int stringList_append(stringList *l, um_fp value) {
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
um_fp stringListView_get(stringListView slv, unsigned int index) {
  stringList temp = (stringList){
      .List_char = stringListView_CharList(slv),
      .List_stringMetaData = stringListView_MetaList(slv),
  };
  return stringList_get(&temp, index);
}
void stringListView_free(stringListView slv) { free(slv.raw.ptr); }
void stringList_free(stringList *l) {
  free(l->List_char.head);
  free(l->List_stringMetaData.head);
  free(l);
}

#endif // STRING_LIST_C
