#ifndef MY_ALLOCATOR_H
#define MY_ALLOCATOR_H
#include <stddef.h>
typedef struct My_allocator My_allocator;
typedef struct My_allocator {
  void *(*alloc)(const My_allocator *, size_t);
  void (*free)(const My_allocator *, void *);
  void *(*r_alloc)(const My_allocator *, void *, size_t);
  void *arb; // state
} My_allocator;

#include <stdlib.h>
static void *default_alloc(const My_allocator *allocator, size_t s) {
  (void)allocator;
  return malloc(s);
}
static void *default_r_alloc(const My_allocator *allocator, void *p, size_t s) {
  (void)allocator;
  return realloc(p, s);
}
static void default_free(const My_allocator *allocator, void *p) {
  (void)allocator;
  return free(p);
}
static const My_allocator defaultAllocator = {
    .alloc = default_alloc,
    .free = default_free,
    .r_alloc = default_r_alloc,
    .arb = NULL,
};
#define aAlloc(allocatorptr, size) ((allocatorptr)->alloc((allocatorptr), size))

#define aRealloc(allocatorptr, voidptr, size) \
  ((allocatorptr)->r_alloc((allocatorptr), voidptr, size))

#define aFree(allocatorptr, voidptr) \
  ((allocatorptr)->free((allocatorptr), voidptr))

#endif // MY_ALLOCATOR_H
