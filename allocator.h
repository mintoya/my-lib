#ifndef MY_ALLOCATOR_H
#define MY_ALLOCATOR_H
#include <stddef.h>
typedef struct My_allocator My_allocator;
typedef void *(*My_allocator_alloc)(const My_allocator *, size_t);
typedef void (*My_allocator_free)(const My_allocator *, void *);
typedef void *(*My_allocator_realloc)(const My_allocator *, void *, size_t);

typedef struct My_allocator {
  My_allocator_alloc alloc;
  My_allocator_free free;
  My_allocator_realloc r_alloc;
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
static const My_allocator *getDefaultAllocator() { return &defaultAllocator; }
#define aAlloc(allocatorptr, size) ((allocatorptr)->alloc((allocatorptr), size))

#define aRealloc(allocatorptr, voidptr, size) \
  ((allocatorptr)->r_alloc((allocatorptr), voidptr, size))

#define aFree(allocatorptr, voidptr) \
  ((allocatorptr)->free((allocatorptr), voidptr))

#endif // MY_ALLOCATOR_H
