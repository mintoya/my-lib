#include "stringList.h"
#include <string.h>
#include <stdlib.h>
#define STRING_LIST_minSize 20
stringList *stringList_new() {
  stringList *res = malloc(sizeof(stringList));
  // clang-format off
  *res = (stringList) {
    .List_stringMetaData = {
        .width = sizeof(stringMetaData),
        .length = 0,
        .size = 1,
        .head = malloc(sizeof(stringMetaData)),
    },

    .List_char = {
      .width = sizeof(char),
      .length = 0,
      .size = 1,
      .head = malloc(sizeof(char)),
    }
  };
  // clang-format on
  return res;
}
#define max(a,b) ((a<b)?(b):(a))

int um_fp_cmp(um_fp a, um_fp b) {
    if (a.width != b.width) { return (a.width < b.width) ? -1 : 1; }
    if (a.width == 0) { return 0; }
    return memcmp(a.ptr, b.ptr, a.width);
}
um_fp stringList_get(stringList *l, unsigned int index) {
  stringMetaData this = mList_get(&(l->List_stringMetaData),stringMetaData,index );
  return ((um_fp){.ptr = (List_gst(&(l->List_char),this.index)),.width=this.width});
}
void stringList_append(stringList *l, um_fp value) {
  unsigned int trueSize = max(value.width,STRING_LIST_minSize);
  stringMetaData this = {
    .index = l->List_char.length,
    .width = value.width,
    ._size = trueSize,
  };
  mList_add(&(l->List_stringMetaData),this);
  List_appendFromArr(&(l->List_char),value.ptr,value.width);
  List_pad(&(l->List_char),trueSize-value.width);
}
void stringList_insert(stringList *l, um_fp value, unsigned int index) {
  unsigned int trueSize = max(value.width,STRING_LIST_minSize);
  stringMetaData this = {
    .index = l->List_char.length,
    .width = value.width,
    ._size = trueSize,
  };
  mList_insert(&(l->List_stringMetaData),this,index);
  List_appendFromArr(&(l->List_char),value.ptr,value.width);
  List_pad(&(l->List_char),trueSize-value.width);
}
void stringList_set(stringList *l, um_fp value, unsigned int index) {
  stringMetaData this = mList_get(&(l->List_stringMetaData),stringMetaData,index );
  if(this._size<value.width){
    stringMetaData* ref = List_gst(&(l->List_stringMetaData),index);
    ref->width = value.width;
    ref->_size = value.width;
    ref->index = l->List_char.length;
    List_appendFromArr(&(l->List_char),value.ptr,value.width); }
  else{
    stringMetaData* ref = List_gst(&(l->List_stringMetaData),index);
    ref->width = value.width;
    memcpy(List_gst(&(l->List_char),ref->index),value.ptr,value.width);
  }
}


void stringList_free(stringList*l){
  free(l->List_char.head);
  free(l->List_stringMetaData.head);
  free(l);
}

