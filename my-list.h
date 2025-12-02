#ifndef MY_LIST_H
#define MY_LIST_H
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static inline bool memzeroed(void *mem, size_t len) {
  for (; len > 0; len--) {
    if (((uint8_t *)mem)[len - 1])
      return false;
  }
  return true;
}
// clang-format off
  #ifndef LIST_GROW_EQ
    #define LIST_GROW_EQ(uint) ((uint + (uint << 1)) + 1)
  #endif
// clang-format on
#include "allocator.h"
typedef struct List {
  size_t width;
  uint32_t length;
  uint32_t size;
  uint8_t *head;
  const My_allocator *allocator;
} List;
typedef enum : uint8_t { CANTRESIZE,
                         INVALID,
                         OK = 0,
} List_opError;
extern inline List_opError List_validState(const List *l) {
  // clang-format off
  return (
      l &&
      l->width &&
      l->size &&
      l->head &&
      l->allocator &&
      l->size >= l->length
  )?OK:INVALID;
  // clang-format on
}
extern inline void List_makeNew(const My_allocator *allocator, List *l, size_t bytes) {
  if (!l)
    return;
  *l = (List){
      .width = bytes,
      .length = 0,
      .size = 1,
      .head = (uint8_t *)aAlloc(allocator, bytes),
      .allocator = allocator,
  };
}
extern inline void List_free(List *l) {
  if (!l || !l->allocator)
    return;
  if (l->head)
    aFree(l->allocator, l->head);
  l->head = NULL;
  l->width = 0;
  aFree(l->allocator, l);
}
// same as list_resize but it enforces size
List_opError List_forceResize(List *l, unsigned int newSize);
extern inline List_opError List_resize(List *l, unsigned int newSize) {
  newSize = newSize ? newSize : 1;
  if ((newSize > l->size || newSize < l->size / 8))
    return List_forceResize(l, newSize);
  return OK;
}
extern inline List *List_new(const My_allocator *allocator, size_t bytes) {
  List *l = (List *)aAlloc(allocator, sizeof(List));
  List_makeNew(allocator, l, bytes);
  return l;
}

[[gnu::pure]] extern inline size_t List_headArea(const List *l) {
  return (l->width * l->length);
}
// all bytes list owns
[[gnu::pure]] extern inline size_t List_fullHeadArea(const List *l) {
  return (l->width * l->size);
}
[[gnu::pure]] extern inline void *List_getRefForce(const List *l, unsigned int i) {
  return (l->head + l->width * i);
}
[[gnu::pure]] extern inline void *List_getRef(const List *l, unsigned int i) {
  if (List_validState(l) != OK)
    return NULL;

  return (i < l->length) ? (l->head + l->width * i) : (NULL);
}

extern inline void List_set(List *l, unsigned int i, const void *element) {
  if (List_validState(l) != OK)
    return;
  if (i < l->length) {
    if (element) {
      memcpy(l->head + i * l->width, element, l->width);
    } else {
      memset(l->head + i * l->width, 0, l->width);
    }
  }
}
List_opError List_append(List *l, const void *element);
List_opError List_insert(List *l, unsigned int i, void *element);
// helper function to append 0's
List_opError List_pad(List *l, unsigned int ammount);
List *List_fromArr(const My_allocator *, const void *source, unsigned int size, unsigned int length);
List_opError List_appendFromArr(List *l, const void *source, unsigned int i);
extern inline uint32_t List_search(const List *l, const void *element) {
  if (!List_validState(l))
    return ((uint32_t)-1);
  unsigned int i = 0;
  if (element) {
    for (; i < l->length; i++) {
      if (!memcmp(element, List_getRef(l, i), l->width))
        return i;
    }
  } else {
    for (; i < l->length; i++) {
      if (memzeroed(List_getRef(l, i), l->width))
        return i;
    }
  }
  return i;
}
extern inline void List_remove(List *l, unsigned int i) {
  if (List_validState(l) != OK)
    return;
  memmove(l->head + i * l->width, l->head + (i + 1) * l->width, (l->length - i - 1) * l->width);
  l->length--;
}
extern inline void List_zeroOut(List *l) {
  if (!List_validState(l))
    return;
  memset(l->head, 0, List_fullHeadArea(l));
}
void *List_toBuffer(List *l);
void *List_fromBuffer(void *ref);
List *List_deepCopy(List *l);
extern inline void List_cleanup_handler(List **l) {
  if (l && *l)
    List_free(*l);
  *l = NULL;
}
// experimental idk
#define List_scoped [[gnu::cleanup(List_cleanup_handler)]] List

#define mList_forEach(list, type, name, ...)               \
  do {                                                     \
    type name;                                             \
    for (unsigned int _i = 0; _i < (list)->length; _i++) { \
      name = (*(type *)List_getRef((list), _i));           \
      __VA_ARGS__                                          \
    }                                                      \
  } while (0)
#define mList(type) List_new(&defaultAllocator, sizeof(type))
#define mList_get(list, type, index) (*(type *)List_getRef(list, index))
#define mList_add(list, type, value)         \
  do {                                       \
    type __val = value;                      \
    List_append(list, (const void *)&__val); \
  } while (0)

#define mList_insert(list, type, value, index) \
  do {                                         \
    type __val = value;                        \
    List_insert(list, index, (void *)&__val);  \
  } while (0)

#define mList_set(list, type, value, index)      \
  do {                                           \
    type __val = value;                          \
    List_set(list, index, (const void *)&__val); \
  } while (0)

#endif // MY_LIST_H

#ifdef MY_LIST_C

List_opError List_insert(List *l, unsigned int i, void *element) {
  if (List_validState(l) != OK)
    return INVALID;
  if (i == l->length)
    return List_append(l, element);
  if (i > l->length)
    return OK;
  if (l->size < l->length + 1) {
    if (List_resize(l, LIST_GROW_EQ(l->length)) != OK)
      return CANTRESIZE;
  }
  if (List_validState(l) != OK)
    return INVALID;
  memmove(l->head + (i + 1) * l->width, l->head + (i)*l->width, (l->length - i) * l->width);
  List_set(l, i, element);
  l->length++;
  return OK;
}
List_opError List_append(List *l, const void *element) {
  List_opError state = OK;
  if (List_validState(l) != OK)
    return INVALID;
  if (l->size < l->length + 1)
    if (List_resize(l, LIST_GROW_EQ(l->length)) != OK)
      return CANTRESIZE;
  if (List_validState(l) != OK)
    return INVALID;
  l->length++;
  List_set(l, l->length - 1, element);
  return OK;
}
List_opError List_forceResize(List *l, unsigned int newSize) {
  uint8_t *newPlace =
      (uint8_t *)aRealloc(l->allocator, l->head, newSize * l->width);
  if (!newPlace) {
    if (newSize < l->size)
      return INVALID;
    else
      return CANTRESIZE;
  } else {
    l->head = newPlace;
    l->size = newSize;
    l->length = (l->length < l->size) ? (l->length) : (l->size);
  }
  return List_validState(l);
}
List_opError List_pad(List *l, unsigned int ammount) {
  if (l->size < l->length + ammount) {
    unsigned int newsize = l->size;
    while (newsize < l->length + ammount) {
      newsize = LIST_GROW_EQ(newsize);
    }
    if (List_resize(l, newsize) != OK)
      return CANTRESIZE;
  }
  memset(l->head + l->length * l->width, 0, ammount * l->width);
  l->length += ammount;
}
List *List_fromArr(const My_allocator *allocator, const void *source, unsigned int width, unsigned int length) {
  List *res = (List *)aAlloc(allocator, sizeof(List));
  res->width = width;
  res->length = length;
  res->size = length;
  res->allocator = allocator;
  res->head = (uint8_t *)aAlloc(allocator, length * width);
  if (res && res->head && source)
    memcpy(res->head, source, length * width);
  return res;
}
List_opError List_appendFromArr(List *l, const void *source, unsigned int ammount) {
  if (List_validState(l) != OK)
    return INVALID;
  if (l->size < l->length + ammount) {
    unsigned int newsize = l->size;
    while (newsize < l->length + ammount) {
      newsize = LIST_GROW_EQ(newsize);
    }
    if (List_resize(l, newsize) != OK)
      return CANTRESIZE;
  }
  memcpy(l->head + l->length * l->width, source, ammount * l->width);
  l->length += ammount;
  return OK;
}

List *List_deepCopy(List *l) {
  return List_fromArr(l->allocator, l->head, l->width, l->length);
}

#endif
