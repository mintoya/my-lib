#ifndef MACROLIST_H
#define MACROLIST_H
#include "my-list.h"

#define MList(type)                                                            \
  struct {                                                                     \
    unsigned int length;                                                       \
    unsigned int capacity;                                                     \
    type *elements;                                                            \
  }
#ifdef __cplusplus
#include <type_traits>
#ifndef typeof
#define typeof(x) std::decay_t<decltype(x)>
#endif
#endif

#define MList_convert(list)                                                    \
  (List){.width = sizeof(typeof(*(list.elements))),                            \
         .length = list.length,                                                \
         .size = list.capacity,                                                \
         .head = (uint8_t *)list.elements};
#define MList_deconvert(name, list)                                            \
  (typeof(list)){                                                              \
      .length = name.length,                                                   \
      .capacity = name.size,                                                   \
      .elements = (typeof(list.elements))name.head,                            \
  };
#define MList_heapList(list) MacroListRef__##list

// automatically freed
#define MList_init(list)                                                       \
  List_scoped *MList_heapList(list) = mList(typeof(list.elements[0]));         \
  list = MList_deconvert((*MList_heapList(list)), list);

#define MList_push(list, ...)                                                  \
  do {                                                                         \
    mList_add(MList_heapList(list), typeof(*(list.elements)), __VA_ARGS__);    \
    list = MList_deconvert((*MList_heapList(list)), list);                     \
  } while (0)

#define MList_pop(list)                                                        \
  ({                                                                           \
    MList_heapList(list)->length--;                                            \
    list = MList_deconvert((*MList_heapList(list)), list);                     \
    list.elements[list.length];                                                \
  })

#endif // MACROLIST_H
