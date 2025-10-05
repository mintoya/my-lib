#ifndef MY_LIST_H
#define MY_LIST_H
#include <errno.h>
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
  unsigned long width;
  unsigned int length;
  unsigned int size;
  char *head;
} List;

List *List_new(unsigned long bytes);

static inline void *List_getRef(const List *l, unsigned int i) {
  void *res;
  res = (i < l->length) ? (l->head + l->width * i) : (NULL);
  return res;
}
static inline void *List_getRefForce(const List *l, unsigned int i) {
  return (l->head + l->width * i);
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
static inline void List_zeroOut(List *l) {
  memset(l->head, 0, l->size * l->width);
}
static inline void List_free(List *l) {
  freAllocate(l->head);
  freAllocate(l);
}
// helper function to pad with 0s
void List_pad(List *l, unsigned int ammount);
List *List_fromArr(const void *source, unsigned int size, unsigned int length);
void List_appendFromArr(List *l, const void *source, unsigned int i);
int List_search(List *l, const void *value);
void List_insert(List *l, unsigned int i, void *element);
void List_remove(List *l, unsigned int i);
void *List_toBuffer(List *l);
void *List_fromBuffer(void *ref);
List *List_deepCopy(List *l);
List *List_combine(List *l, List *l2);
#ifndef __cplusplus
#define mList_forEach(list, type, item, scope)                                 \
  {                                                                            \
    type item;                                                                 \
    for (unsigned int _i = 0; _i < (list)->length; _i++) {                     \
      item = *((type *)List_getRef((list), _i));                               \
      scope                                                                    \
    }                                                                          \
  }
#define mList_T(type) List_new(sizeof(type))
#define mList_get(list, type, index) *(type *)List_getRef(list, index)
#define mList_add(list, type, ...) List_append(list, (type[1]){__VA_ARGS__})
#define mList_insert(list, type, value, index)                                 \
  List_insert(list, index, (type[1]){value})

#define mList(type, ...)                                                       \
  List_fromArr((type[]){__VA_ARGS__}, sizeof(type),                            \
               sizeof((type[]){__VA_ARGS__}) / sizeof(type))
#endif

#endif

#ifdef MY_LIST_C

List *List_new(unsigned long bytes) {
  List *l = (List *)clearAllocate(1, sizeof(List));
  *l = (List){
      .width = bytes,
      .length = 0,
      .size = 1,
      .head = (char *)regularAllocate(bytes),
  };
  return l;
}
// same as list_resize but it enforces size
void List_forceResize(List *l, unsigned int newSize) {
  char *newPlace = (char *)reAllocate(l->head, newSize * l->width);
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
  char *newPlace = (char *)reAllocate(l->head, newSize * l->width);
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
  memset(l->head + l->length * l->width, 0, l->size-l->length * l->width);
  l->length += ammount;
}
List *List_fromArr(const void *source, unsigned int width,
                   unsigned int length) {
  List *res = (List *)regularAllocate(sizeof(List));
  res->width = width;
  res->length = length;
  res->size = length;
  res->head = (char *)regularAllocate(length * width);
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
