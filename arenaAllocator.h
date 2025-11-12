#ifndef ARENA_ALLOCATOR_H
#define ARENA_ALLOCATOR_H
#include "allocator.h"
#include "print.h"
#include <errno.h>
#include <stdint.h>
#include <string.h>

typedef struct ArenaBlock ArenaBlock;
typedef struct ArenaBlock {
  ArenaBlock *next;
  size_t place;
  size_t size;
  uint8_t arr[];
} ArenaBlock;

ArenaBlock *arenablock_new(size_t blockSize);
void *arena_alloc(const My_allocator *ref, size_t size);
void *arena_alloc(const My_allocator *ref, size_t size);
static void arena_free(const My_allocator *allocator, void *ptr) {
  // noop
}
void *arena_r_alloc(const My_allocator *arena, void *ptr, size_t size);
My_allocator *arena_new(size_t blockSize);
void arenablock_free(ArenaBlock *block);
void arena_cleanup(My_allocator *arena);
static void arena_cleanup_handler(My_allocator **arenaPtr) {
  if (arenaPtr && *arenaPtr)
    arena_cleanup(*arenaPtr);
  *arenaPtr = NULL;
}
#define Arena_scoped [[gnu::cleanup(arena_cleanup_handler)]] My_allocator
#endif // ARENA_ALLOCATOR_H
#ifdef ARENA_ALLOCATOR_C
void arena_cleanup(My_allocator *arena) {
  ArenaBlock *it = (ArenaBlock *)(arena->arb);
  arenablock_free(it);
  aFree((&defaultAllocator), arena);
}
void arenablock_free(ArenaBlock *block) {
  while (block) {
    ArenaBlock *next = block->next;
    // println("free block of size ${}", block->size);
    aFree((&defaultAllocator), block);
    block = next;
  }
}
My_allocator *arena_new(size_t blockSize) {
  My_allocator *res =
      (My_allocator *)aAlloc((&defaultAllocator), sizeof(My_allocator));
  if (!res)
    exit(ENOMEM);
  *res = (My_allocator){
      .alloc = arena_alloc,
      .free = arena_free,
      .r_alloc = arena_r_alloc,
      .arb = arenablock_new(blockSize),
  };
  return res;
}
void *arena_r_alloc(const My_allocator *arena, void *ptr, size_t size) {
  size_t *lastSize = (size_t *)((uint8_t *)ptr - sizeof(size_t));
  if (*lastSize > size)
    return ptr;
  void *res = arena_alloc(arena, size);
  memmove(res, ptr, (*lastSize));
  return res;
}
void *arena_alloc(const My_allocator *ref, size_t size) {
  ArenaBlock *it = (ArenaBlock *)(ref->arb);
  void *res = NULL;
  while (!res) {
    if (it->size - it->place >= size + sizeof(size_t)) {
      *(size_t *)(it->arr + it->place) = size;
      it->place += sizeof(size_t);

      res = it->arr + it->place;
      it->place += size;
    } else {
      if (!it->next) {
        int minsize = size + sizeof(size_t);
        minsize = (minsize < it->size ? it->size : minsize);
        it->next = arenablock_new(minsize);
      }
      it = it->next;
    }
  }
  return res;
}
ArenaBlock *arenablock_new(size_t blockSize) {
  ArenaBlock *res = (ArenaBlock *)aAlloc((&defaultAllocator),
                                         (sizeof(ArenaBlock) + blockSize));
  if (!res)
    exit(ENOMEM);
  *res = (ArenaBlock){
      .next = NULL,
      .place = 0,
      .size = blockSize,
  };
  return res;
}
#endif // ARENA_ALLOCATOR_C
