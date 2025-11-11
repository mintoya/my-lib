#ifndef MY_ALLOCATOR_H
#define MY_ALLOCATOR_H
#include <stddef.h>
typedef struct My_allocator {
  void *(*alloc)(const struct My_allocator *, size_t);
  void (*free)(const struct My_allocator *, void *);
  void *(*r_alloc)(const struct My_allocator *, void *, size_t);
  void *arb;
} My_allocator;

#include <stdlib.h>
void *default_alloc(const My_allocator *allocator, size_t s) {
  return malloc(s);
}
void *default_r_alloc(const My_allocator *allocator, void *p, size_t s) {
  return realloc(p, s);
}
void default_free(const My_allocator *allocator, void *p) { return free(p); }
static const My_allocator defaultAllocator = {
    .alloc = default_alloc,
    .free = default_free,
    .r_alloc = default_r_alloc,
    .arb = NULL,
};
#define aAlloc(allocatorptr, size) allocatorptr->alloc(allocatorptr, size)
#define aRealloc(allocatorptr, voidptr, size)                                  \
  allocatorptr->r_alloc(allocatorptr, voidptr, size)
#define aFree(allocatorptr, voidptr) allocatorptr->free(allocatorptr, voidptr)

// arena

// typedef struct ArenaAlocObj;
// typedef struct ArenaAlocObj {
//   void *start;
//   size_t place;
//   size_t capacity;
//   ArenaAlocObj *next;
// } ArenaAlocObj;
// void *ArenaAloc_alloc(const My_allocator *ts, size_t size) {}
// void *ArenaAloc_r_alloc(const My_allocator *ts, void *which, size_t size) {}
// void ArenaAloc_free(const My_allocator *ts, void *) {}
// My_allocator Arena_new(size_t bucketSize) {
//   My_allocator res;
//   ArenaAlocObj *arena =
//       (ArenaAlocObj *)aAlloc((&defaultAllocator), sizeof(ArenaAlocObj));
//   *arena = (ArenaAlocObj){
//       .start = aAlloc((&defaultAllocator), bucketSize),
//       .place = 0,
//       .capacity = bucketSize,
//   };
//   res = (My_allocator){
//       .alloc = ArenaAloc_alloc,
//       .free = ArenaAloc_free,
//       .r_alloc = ArenaAloc_r_alloc,
//       .arb = arena,
//   };
//   return res;
// }
#endif // MY_ALLOCATOR_H
