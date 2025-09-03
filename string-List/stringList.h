#ifndef STRING_LIST_H
#define STRING_LIST_H
#define initialBufferSize
#include "../my-list/my-list.h"
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

typedef struct fat_pointer {
  void *ptr;
  size_t width;
} um_fp;
int um_fp_cmp(um_fp a, um_fp b);
stringList *stringList_new();
um_fp stringList_get(stringList *l, unsigned int index);
void stringList_insert(stringList *l, um_fp, unsigned int index);
void stringList_set(stringList *l, um_fp, unsigned int index);
void stringList_append(stringList *l, um_fp);
void stringList_free(stringList *l);
static inline unsigned int stringList_length(stringList*l){
  return l->List_stringMetaData.length;
}
#define nullUmf ((um_fp){.ptr = NULL, .width = 0})
#define um_from(val)                                                           \
  _Generic((val),                                                              \
      char *: (um_fp){.ptr = (val), .width = strlen(val)},                     \
      const char *: (um_fp){.ptr = (val), .width = strlen(val)},               \
      default: (um_fp){.ptr = (typeof(val)[1]){val},                           \
                       .width = sizeof(typeof(val))}) // structs

#endif // STRING_LIST_H
