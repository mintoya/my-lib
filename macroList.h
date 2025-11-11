// #ifdef __cplusplus
// #error macrolist.h not supported in cpp, just use the class
// #else
#ifndef MACROLIST_H
#define MACROLIST_H
#include "my-list.h"

#define MList(type)                                                            \
  struct {                                                                     \
    unsigned int length;                                                       \
    type *elements;                                                            \
  }
#define MList_t(type) MList_typedef_##type
#define MList_typeDef(type) typedef MList(type) MList_t(type);

#define MList_deconvert(name, list)                                            \
  (typeof(list)){                                                              \
      .length = name.length,                                                   \
      .elements = (typeof(list.elements))name.head,                            \
  };
#define MList_heapList(list) __MacroListRef__##list
#define MList_dnfList(list) __MacroListTempRef##list

#define MList_castT(type, list)                                                \
  ((MList_t(type)){.length = list.length, .elements = list.elements})

// automatically freed
#define MList_init(list)                                                       \
  List_scoped *MList_heapList(list) =                                          \
      List_new(&defaultAllocator, sizeof(typeof(list.elements[0])));           \
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

#define MList_foreach(list, index, element, ...)                               \
  do {                                                                         \
    for (size_t index = 0; index < list.length; index++) {                     \
      typeof(list.elements[0]) element = list.elements[index];                 \
      __VA_ARGS__                                                              \
    }                                                                          \
  } while (0)
#define MList_addArr(list, size, ...)                                          \
  do {                                                                         \
    MList_heapList(list)->length = list.length;                                \
    typeof(list.elements) __temp = (typeof(list.elements))__VA_ARGS__;         \
    void *__ref = (void *)__temp;                                              \
    List_appendFromArr(MList_heapList(list), __ref, size);                     \
    list = MList_deconvert((*MList_heapList(list)), list);                     \
  } while (0)

#define MList_capacity(list) MList_heapList(list)->size

// if you want to control the scope you have to provide a list pointer
#define MList_DFInit(list, Listptr)                                            \
  Listptr = List_new(&defaultAllocator, sizeof(typeof(list.elements[0])));     \
  List *MList_heapList(list) = Listptr;                                        \
  list = MList_deconvert((*MList_heapList(list)), list);
#define MList_DF(list, type, Listptr)                                          \
  List *MList_heapList(list) = Listptr;                                        \
  MList(type) list;                                                            \
  list = MList_deconvert((*MList_heapList(list)), list);

#define MList_fp(list)                                                         \
  ((um_fp){.ptr = (uint8_t *)list.elements,                                    \
           .width = list.length * sizeof(typeof(list.elements[0]))})

#endif // MACROLIST_H
// #endif // cpp
