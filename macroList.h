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

// automatically freed
#define MList_init(list)                                                       \
  List_scoped *_MacroListRef__##list = mList(typeof(list.elements[0]));        \
  list = MList_deconvert((*_MacroListRef__##list), list);

#define MList_push(list, ...)                                                  \
  do {                                                                         \
    List tempList = MList_convert(list);                                       \
    List *tempRef = &tempList;                                                 \
    mList_add(tempRef, typeof(*(list.elements)), __VA_ARGS__);                 \
    list = MList_deconvert(tempList, list);                                    \
  } while (0)

#define MList_pop(list)                                                        \
  ({                                                                           \
    list.length--;                                                             \
    list.elements[list.length];                                                \
  })

#endif // MACROLIST_H
