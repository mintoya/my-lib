#pragma once
#include "my-list.h"

template <typename T> struct listPlus {
  List *ptr;

  listPlus() { ptr = List_new(sizeof(T)); }

  // listPlus(void *p) { ptr = (List *)p; }
  listPlus(List *p) { ptr = p; }

  listPlus(T *arr, unsigned int length) {
    ptr = List_new(sizeof(T));
    List_fromArr(ptr, arr, length);
  }
  T *self() { return (T *)ptr->head; }
  const unsigned int length() { return ptr->length; }

  ~listPlus() { List_free(ptr); }

  void pad(unsigned int ammount) { List_pad(ptr, ammount); }
  void resize(unsigned int newSize) { List_resize(ptr, newSize); }
  void unmake() { List_free(ptr); }

  inline void append(const T &value) { List_append(ptr, (void *)&value); }
  inline void push(const T &value) { List_append(ptr, (void *)&value); }
  inline T get(unsigned int i) { return *((T *)List_getRef(ptr, i)); }
  T pop() {
    ptr->length--;
    return *((T *)List_getRefForce(ptr, ptr->length));
  }

  void appendArr(const T values[], unsigned int len) {
    List_appendFromArr(ptr, values, len);
  }

  void insert(const T &value, int index) {
    List_insert(ptr, index, (void *)&value);
  }

  void set(unsigned int i, const T &value) { List_set(ptr, i, &value); }
  int searchFor(const T &value) { return List_search(ptr, &value); }
  inline void clear() { ptr->length = 0; }
  inline unsigned int length() const { return ptr->length; }
};
