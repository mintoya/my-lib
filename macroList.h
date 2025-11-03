#include "my-list.h"

#define MList(type)                                                            \
  struct {                                                                     \
    unsigned int length;                                                       \
    unsigned int size;                                                         \
    type *elements;                                                            \
  }
#ifdef __cplusplus
#include <type_traits> // Add this include

#define typeof(x) std::decay_t<decltype(x)>
#endif

#define MList_convert(list)                                                    \
  (List){.width = sizeof(typeof(*(list.elements))),                            \
         .length = list.length,                                                \
         .size = list.size,                                                    \
         .head = (uint8_t *)list.elements};
#define MList_deconvert(name, list)                                            \
  (typeof(list)){                                                              \
      .length = name.length,                                                   \
      .size = name.size,                                                       \
      .elements = (typeof(list.elements))name.head,                            \
  };

#define MList_init(list)                                                       \
  List tempList = MList_convert(list);                                         \
  List_scoped *tempRef##list = mList(typeof(list.elements[0]));                \
  list = MList_deconvert(tempList, list);

#define MList_append(list, value)                                              \
  do {                                                                         \
    List tempList = MList_convert(list);                                       \
    List *tempRef = &tempList;                                                 \
    mList_add(tempRef, typeof(*(list.elements)), value);                       \
    list = MList_deconvert(tempList, list);                                    \
  } while (0)

// int main() {
//   MList(int) listn;
//   MList_init(listn);
//   MList_append(listn, 1);
//
//   listn.elements[0];
// }
