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

extern const My_allocator *defaultAlloc;
extern inline void *aAlloc(const My_allocator *allocator, size_t size);
extern inline void *aRealloc(const My_allocator *allocator, void *oldptr, size_t size);
extern inline void aFree(const My_allocator *allocator, void *oldptr);
// object-like allocation
#define aAlloc(allocator, size)        \
  ({                                   \
    allocator->alloc(allocator, size); \
  })
#define aRealloc(allocator, oldptr, size)        \
  ({                                             \
    allocator->r_alloc(allocator, oldptr, size); \
  })
#define aFree(allocator, oldptr)        \
  ({                                    \
    allocator->free(allocator, oldptr); \
  })
#endif // MY_ALLOCATOR_H
#ifdef MY_ALLOCATOR_C
#include <stdlib.h>
void *default_alloc(const My_allocator *allocator, size_t s) {
  (void)allocator;
  return malloc(s);
}
void *default_r_alloc(const My_allocator *allocator, void *p, size_t s) {
  (void)allocator;
  return realloc(p, s);
}
void default_free(const My_allocator *allocator, void *p) {
  (void)allocator;
  return free(p);
}
#include "fptr.h"
const My_allocator defaultAllocator = (My_allocator){default_alloc, default_free, default_r_alloc};
const My_allocator *defaultAlloc = &defaultAllocator;
#endif // MY_ALLOCATOR_C
