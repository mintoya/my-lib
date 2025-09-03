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
  char *head;
} List;

#define typedList(type)                                                        \
  struct {                                                                     \
    unsigned long width;                                                       \
    unsigned int length;                                                       \
    unsigned int size;                                                         \
    type *head;                                                                \
  } List_##type
#define typedList_init(typedList)                                              \
  (List_t_##type) {                                                            \
    .width = sizeof(typedList.head[0]), .length = 0, .size = 1,                \
    .head = regularAllocate(sizeof(typedList.head[0]))                         \
  }

List *List_new(unsigned long bytes);
void List_set(List *l, unsigned int i, const void *element);
void *List_gst(const List *l, unsigned int i);
void List_resize(List *l, unsigned int newSize);
void List_append(List *l, const void *element);
void List_pad(List*l,unsigned int ammount);
List *List_fromArr(const void *source, unsigned int size, unsigned int length);
void List_appendFromArr(List *l, const void *source, unsigned int i);
int List_search(List *l, const void *value);
void List_insert(List *l, unsigned int i, void *element);
void List_remove(List *l, unsigned int i);
void List_zeroOut(List *l);
void List_free(List *l);
void List_print(const List *l);
int List_filter(List *l, int (*function)(void *));
void *List_toBuffer(List *l);
/*
 * fromBuffer is the inverse of toBuffer
 * cannot be list_freed
 * does not allocate memory
 */
void *List_fromBuffer(void *ref);
List *List_deepCopy(List *l);
List *List_combine(List *l, List *l2);
#define mList_forEach(list, type, item, scope)                                 \
  {                                                                            \
    type item;                                                                 \
    for (unsigned int _i = 0; _i < (list)->length; _i++) {                     \
      item = *((type *)List_gst((list), _i));                                  \
      scope                                                                    \
    }                                                                          \
  }

#define mList_get(list, type, index) *(type *)List_gst(list, index)
#define mList_add(list, value) List_append(list, (typeof(value)[1]){value})
#define mList_insert(list, value,index) List_insert(list, index, (typeof(value)[1]){value})

// Actual implementations
#define mList_fromType(type) List_new(sizeof(type))
#define mList_fromList(type, array)                                            \
  List_fromArr(array, sizeof(type), sizeof(array) / sizeof(type))

// Macro to select correct macro based on argument count
#define _GET_MACRO(_1, _2, NAME, ...) NAME

// Dispatcher macro
#define mList(...)                                                             \
  _GET_MACRO(__VA_ARGS__, mList_fromList, mList_fromType)(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
