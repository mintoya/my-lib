#ifndef MY_LIST_H
#define MY_LIST_H
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
// clang-format off
#ifndef LIST_ALLOCATOR
  #define clearAllocate(length, size) calloc(length, size)
  #define regularAllocate(size) malloc(size)
  #define reAllocate(source, newsize) realloc(source, newsize)
  #define freAllocate(ptr) free(ptr)
#endif
// clang-format on

// clang-format off
  #ifndef LIST_GROW_EQ
    #define LIST_GROW_EQ(uint) ((uint + (uint << 1)) + 1)
  #endif
// clang-format on
#include <string.h>
typedef struct List {
  size_t width;
  unsigned int length;
  unsigned int size;
  uint8_t *head;
} List;

List *List_new(size_t bytes);
// valid bytes list owns
static inline size_t List_headArea(const List *l) {
  return (l->width * l->length);
}
// all bytes list owns
static inline size_t List_fullHeadArea(const List *l) {
  return (l->width * l->size);
}
static inline void *List_getRefForce(const List *l, unsigned int i) {
  return (l->head + l->width * i);
}
static inline void *List_getRef(const List *l, unsigned int i) {
  void *res;
  res = (i < l->length) ? (l->head + l->width * i) : (NULL);
  return res;
}
void List_resize(List *l, unsigned int newSize);
void List_forceResize(List *l, unsigned int newSize);

static inline void List_set(List *l, unsigned int i, const void *element) {
  if (i < l->length) {
    memcpy(l->head + i * l->width, element, l->width);
  }
  // else {
  //   exit(1);
  // }
}
static inline void List_append(List *l, const void *element) {
  if (l->size < l->length + 1) {
    List_resize(l, LIST_GROW_EQ(l->length));
  }
  l->length++;
  List_set(l, l->length - 1, element);
}
// helper function to pad with 0s
void List_pad(List *l, unsigned int ammount);

List *List_fromArr(const void *source, unsigned int size, unsigned int length);
void List_appendFromArr(List *l, const void *source, unsigned int i);
int List_search(List *l, const void *value);
void List_insert(List *l, unsigned int i, void *element);
void List_remove(List *l, unsigned int i);
static inline void List_zeroOut(List *l) {
  memset(l->head, 0, List_fullHeadArea(l));
}
void List_free(List *l);
void *List_toBuffer(List *l);
void *List_fromBuffer(void *ref);
List *List_deepCopy(List *l);
List *List_combine(List *l, List *l2);
static inline void List_cleanup_handler(List **l) {
  if (l && *l)
    List_free(*l);
  *l = NULL;
}
// experimental idk
#define List_scoped List __attribute__((cleanup(List_cleanup_handler)))

#define mList_forEach(list, type, name, ...)                                   \
  do {                                                                         \
    type name;                                                                 \
    for (unsigned int _i = 0; _i < (list)->length; _i++) {                     \
      name = (*(type *)List_getRef((list), _i));                               \
      __VA_ARGS__                                                              \
    }                                                                          \
  } while (0)
#define mList(type) List_new(sizeof(type))
#define mList_get(list, type, index) (*(type *)List_getRef(list, index))
#define mList_add(list, type, value)                                           \
  do {                                                                         \
    type __val = value;                                                        \
    List_append(list, (const void *)&__val);                                   \
  } while (0)

#define mList_insert(list, type, value, index)                                 \
  do {                                                                         \
    type __val = value;                                                        \
    List_insert(list, index, (void *)&__val);                                  \
  } while (0)

#define mList_set(list, type, value, index)                                    \
  do {                                                                         \
    type __val = value;                                                        \
    List_set(list, index, (const void *)&__val);                               \
  } while (0)

#endif // MY_LIST_H

#ifdef MY_LIST_C

#include <stdio.h>
List *List_new(size_t bytes) {
  // printf("new list with %i size", bytes);
  // fflush(stdout);
  List *l = (List *)clearAllocate(1, sizeof(List));
  *l = (List){
      .width = bytes,
      .length = 0,
      .size = 1,
      .head = (uint8_t *)regularAllocate(bytes),
  };
  return l;
}
void List_free(List *l) {
  freAllocate(l->head);
  freAllocate(l);
}
// same as list_resize but it enforces size
void List_forceResize(List *l, unsigned int newSize) {
  uint8_t *newPlace = (uint8_t *)reAllocate(l->head, newSize * l->width);
  if (!newPlace) {
    exit(ENOMEM); // maybe something else idk
  }
  l->head = newPlace;
  l->size = newSize;
  l->length = (l->length < l->size) ? (l->length) : (l->size);
}
void List_resize(List *l, unsigned int newSize) {
  if (newSize <= l->size)
    return;
  uint8_t *newPlace = (uint8_t *)reAllocate(l->head, newSize * l->width);
  if (!newPlace) {
    exit(ENOMEM); // maybe something else idk
  }
  l->head = newPlace;
  l->size = newSize;
  // never resizes down
  // l->length = (l->length < l->size) ? (l->length) : (l->size);
}

void List_pad(List *l, unsigned int ammount) {
  if (l->size < l->length + ammount) {
    unsigned int newsize = l->size;
    while (newsize < l->length + ammount) {
      newsize = LIST_GROW_EQ(newsize);
    }
    List_resize(l, newsize);
  }
  memset(l->head + l->length * l->width, 0, ammount * l->width);
  l->length += ammount;
}
List *List_fromArr(const void *source, unsigned int width,
                   unsigned int length) {
  List *res = (List *)regularAllocate(sizeof(List));
  res->width = width;
  res->length = length;
  res->size = length;
  res->head = (uint8_t *)regularAllocate(length * width);
  memcpy(res->head, source, length * width);
  return res;
}
void List_appendFromArr(List *l, const void *source, unsigned int ammount) {
  if (l->size < l->length + ammount) {
    unsigned int newsize = l->size;
    while (newsize < l->length + ammount) {
      newsize = LIST_GROW_EQ(newsize);
    }
    List_resize(l, newsize);
  }
  memcpy(l->head + l->length * l->width, source, ammount * l->width);
  l->length += ammount;
}
void List_insert(List *l, unsigned int i, void *element) {
  if (i == l->length)
    return List_append(l, element);
  if (i > l->length)
    return; // maybe exit
  if (l->size < l->length + 1) {
    List_resize(l, LIST_GROW_EQ(l->length));
  }
  memmove(l->head + (i + 1) * l->width, l->head + (i)*l->width,
          (l->length - i) * l->width);
  List_set(l, i, element);
  l->length++;
}
void List_remove(List *l, unsigned int i) {

  memmove(l->head + i * l->width, l->head + (i + 1) * l->width,
          (l->length - i - 1) * l->width);
  l->length--;
}

List *List_deepCopy(List *l) {
  return List_fromArr(l->head, l->width, l->length);
}

// returns -1 if item not found
int List_search(List *l, const void *value) {
  for (unsigned int i = 0; i < l->length; i++) {
    if (!memcmp(value, List_getRef(l, i), l->width))
      return i;
  }
  return -1;
}
#endif
