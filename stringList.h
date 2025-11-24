#ifndef STRING_LIST_H
#define STRING_LIST_H
#include "allocator.h"
#include "boxer.h"
#include "fptr.h"
#include "my-list.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  unsigned int index;
  unsigned int width;
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

#define advance(dst, src, length) \
  memcpy(dst, src, length);       \
  dst += length / sizeof(uint8_t);
// memory layot:
//  { size_t:keycount | metadata buffer | data buffer }
typedef struct {
  unsigned int metaSize;
  stringMetaData *Arr_stringMetaData;
  uint8_t *Arr_char;
} stringList_Solid;
const Boxer stringListBoxer = {2, {{.type = FPTR}, {.type = FPTR}}};
static inline stringListView stringList_toView(const My_allocator *allocator, stringList *sl) {
  fptr meta = (fptr){List_headArea(&(sl->List_stringMetaData)), sl->List_stringMetaData.head};
  fptr buff = (fptr){List_headArea(&(sl->List_char)), sl->List_char.head};
  void *bx[2] = {&(meta), &(buff)};
  fptr res = enBox(allocator, &stringListBoxer, bx);

  return (stringListView){res};
}
static inline stringList_Solid stringList_fromView(stringListView slv) {
  stringList_Solid res;
  bFptr *meta, *buff;
  void *bx[2] = {&(meta), &(buff)};
  unBox(&stringListBoxer, bx, slv.raw);
  return (stringList_Solid){
      .metaSize = (unsigned int)(meta->width / sizeof(stringMetaData)),
      .Arr_stringMetaData = (stringMetaData *)meta->ptr,
      .Arr_char = (uint8_t *)buff->ptr,
  };
}
static inline int stringListView_length(stringListView slv) {
  // meta is first
  size_t size = *(size_t *)slv.raw.ptr;
  return size / sizeof(stringMetaData);
}
static inline fptr stringListView_get(stringListView slv, unsigned int index) {
  stringList_Solid sls = stringList_fromView(slv);
  if (index > sls.metaSize)
    return nullFptr;
  stringMetaData thisS = sls.Arr_stringMetaData[index];
  return ((fptr){thisS.width, (uint8_t *)(sls.Arr_char + thisS.index)});
}

void stringListView_free(const My_allocator *allocator, stringListView slv);

// returns length if not found
unsigned int stringList_search(stringList *l, fptr key);
// new sl with unused space removed
stringList *stringList_remake(stringList *);
void stringList_free(stringList *l);
[[gnu::pure]] static inline unsigned int stringList_length(stringList *l) {
  return l->List_stringMetaData.length;
}
static inline void stringList_cleanup_handler(stringList **sl) {
  if (sl && *sl)
    stringList_free(*sl);
  *sl = NULL;
}

#define stringList_scoped \
  [[gnu::cleanup(stringList_cleanup_handler)]] stringList

#endif // STRING_LIST_H

#ifdef STRING_LIST_C
#include <string.h>

stringList *stringList_new(const My_allocator *allocator) {
  stringList *res = (stringList *)aAlloc(allocator, sizeof(stringList));
  List_makeNew(allocator, &(res->List_char), 1);
  List_makeNew(allocator, &(res->List_stringMetaData), sizeof(stringMetaData));
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
  stringMetaData thisS = {
      .index = l->List_char.length,
      .width = value.width,
  };
  mList_add(&(l->List_stringMetaData), stringMetaData, thisS);
  List_appendFromArr(&(l->List_char), value.ptr, value.width);
  return l->List_stringMetaData.length - 1;
}
void stringList_insert(stringList *l, fptr value, unsigned int index) {
  stringMetaData thisS = {
      .index = l->List_char.length,
      .width = value.width,
  };
  mList_insert(&(l->List_stringMetaData), stringMetaData, thisS, index);
  List_appendFromArr(&(l->List_char), value.ptr, value.width);
}
void stringList_set(stringList *l, fptr value, unsigned int index) {
  if (index == stringList_length(l)) {
    stringList_append(l, value);
    return;
  }
  stringMetaData thisS =
      mList_get(&(l->List_stringMetaData), stringMetaData, index);
  if (thisS.width < value.width) {
    stringMetaData *ref =
        (stringMetaData *)List_getRef(&(l->List_stringMetaData), index);
    ref->width = value.width;
    ref->index = l->List_char.length;
    List_appendFromArr(&(l->List_char), value.ptr, value.width);
  } else {
    stringMetaData *ref =
        (stringMetaData *)List_getRef(&(l->List_stringMetaData), index);
    ref->width = value.width;
    memcpy(List_getRef(&(l->List_char), ref->index), value.ptr, value.width);
  }
}

void stringListView_free(const My_allocator *allocator, stringListView slv) { aFree(allocator, slv.raw.ptr); }
void stringList_free(stringList *l) {
  const My_allocator *allocator = l->List_char.allocator;
  aFree(allocator, l->List_char.head);
  aFree(allocator, l->List_stringMetaData.head);
  aFree(allocator, l);
}

#endif // STRING_LIST_C
