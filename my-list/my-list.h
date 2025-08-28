// list.h
#ifndef LIST_f064fdac_a5b9_4a80_9e04_5a62c5d3bd19_H
#define LIST_f064fdac_a5b9_4a80_9e04_5a62c5d3bd19_H
#ifdef __cplusplus
extern "C" {
#endif

#include "../alloc.h"
// #ifdef REGULARALLOCATE
//   #include <stdlib.h>
//   #define clearAllocate(length,size) calloc(length,size)
//   #define regularAllocate(size) malloc(size)
//   #define reAllocate(source,newsize) realloc(source, newSize)
//   #define freAllocate(ptr) free(ptr)
// #endif

typedef struct List {
  unsigned long width;
  unsigned int length;
  unsigned int size;
  void *head;
} List;
typedef struct LL {
  List *l;
} LL;
List *List_new(unsigned long bytes);
void List_set(List *l, unsigned int i, const void *element);
void *List_gst(const List *l, unsigned int i);
void List_resize(List *l, unsigned int newSize);
void List_append(List *l, const void *element);
List* List_fromArr(const void *source, unsigned int size,unsigned int length);
void List_appendFromArr(List *l, const void *source, unsigned int i);
int List_search(List *l, const void *value);
void List_insert(List *l, unsigned int i, void *element);
void List_remove(List *l, unsigned int i);
void List_zeroOut(List *l);
void List_free(List *l);
void List_print(const List*l);
int List_filter(List *l, int (*function)(void *));
void* List_toBuffer(List*l);
/*
 * fromBuffer is the inverse of toBuffer
 * cannot be list_freed
 * does not allocate memory
 */
void* List_fromBuffer(void*ref);
List* List_deepCopy(List*l);
List* List_combine(List*l,List*l2);
#define mList_forEach(list, type, item, scope)                                 \
  {                                                                            \
    type item;                                                                 \
    for (unsigned int _i = 0; _i < (list)->length; _i++) {                     \
      item = *((type *)List_gst((list), _i));                                  \
      scope                                                                    \
    }                                                                          \
  }

#define mList_get(list,type,index) *(type*)List_gst(list,index)
#define mList(type) List_new(sizeof(type))
#define mList_(array) List_fromArr(array,sizeof(array[0]),sizeof(array)/sizeof(array[0]));


#ifdef __cplusplus
}
#endif

#endif
