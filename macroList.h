#ifdef __cplusplus
#error macrolist.h not supported in cpp, just use the class
#else
#ifndef MACROLIST_H
#define MACROLIST_H
#include "my-list.h"

#define MList(type)                                                            \
  struct {                                                                     \
    unsigned int length;                                                       \
    type *elements;                                                            \
  }

#define MList_deconvert(name, list)                                            \
  (typeof(list)){                                                              \
      .length = name.length,                                                   \
      .elements = (typeof(list.elements))name.head,                            \
  };
#define MList_heapList(list) __MacroListRef__##list

// automatically freed
#define MList_init(list)                                                       \
  List_scoped *MList_heapList(list) = List_new(sizeof(typeof(list.elements[0])));     \
  list = MList_deconvert((*MList_heapList(list)), list);

// not automatically freed
#define MList_initDF(list)                                                     \
  List *MList_heapList(list) = List_new(sizeof(typeof(list.elements[0])));     \
  list = MList_deconvert((*MList_heapList(list)), list);

#define MList_push(list, ...)                                                  \
  do {                                                                         \
    MList_heapList(list)->length = list.length;                                \
    typeof(*(list.elements)) __val = __VA_ARGS__;                              \
    List_append(MList_heapList(list), &__val);                                 \
    list = MList_deconvert((*MList_heapList(list)), list);                     \
  } while (0)

#define MList_pop(list)                                                        \
  ({                                                                           \
    MList_heapList(list)->length--;                                            \
    list = MList_deconvert((*MList_heapList(list)), list);                     \
    list.elements[list.length];                                                \
  })

#define MList_insert(list, index, ...)                                         \
  do {                                                                         \
    MList_heapList(list)->length = list.length;                                \
    typeof(*(list.elements)) __val = __VA_ARGS__;                              \
    List_insert(MList_heapList(list), index, (void *)&__val);                  \
    list = MList_deconvert((*MList_heapList(list)), list);                     \
  } while (0)

#define MList_foreach(list, element, ...)                                      \
  do {                                                                         \
    for (size_t _i = 0; _i < list.length; _i++) {                              \
      typeof(list.elements[0]) element = list.elements[_i];                    \
      __VA_ARGS__                                                              \
    }                                                                          \
  } while (0)

#define MList_capacity(list) MList_heapList(list)->size

#endif // MACROLIST_H
#endif // cpp
