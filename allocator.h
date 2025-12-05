#ifndef MY_ALLOCATOR_H
#define MY_ALLOCATOR_H
#include <stddef.h>

typedef struct My_allocator My_allocator;
typedef void *(*My_allocatorAlloc)(const My_allocator *, size_t);
typedef void (*My_allocatorFree)(const My_allocator *, void *);
typedef void *(*My_allocatorRealloc)(const My_allocator *, void *, size_t);

typedef My_allocator *(*OwnAllocatorInit)(void);
typedef void (*OwnAllocatorDeInit)(My_allocator *);

typedef struct My_allocator {
  My_allocatorAlloc alloc;
  My_allocatorFree free;
  My_allocatorRealloc r_alloc;
  void *arb; // state
} My_allocator;

typedef struct {
  OwnAllocatorInit init;
  OwnAllocatorDeInit deInit;
} Own_Allocator;

typedef My_allocator MyAllocator;
typedef Own_Allocator OwnAllocator;

#include <stdlib.h>
static void *
default_alloc(const My_allocator *allocator, size_t s) {
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
// object-like allocation
static inline void *aAlloc(const My_allocator *allocator, size_t size) {
  if (!allocator)
    return default_alloc(NULL, size);
  return allocator->alloc(allocator, size);
}
static inline void *aRealloc(const My_allocator *allocator, void *oldptr, size_t size) {
  if (!allocator)
    return default_r_alloc(NULL, oldptr, size);
  return allocator->r_alloc(allocator, oldptr, size);
}
static inline void aFree(const My_allocator *allocator, void *oldptr) {
  if (!allocator)
    return default_free(NULL, oldptr);
  return allocator->free(allocator, oldptr);
}
#endif // MY_ALLOCATOR_H
