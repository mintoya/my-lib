#ifndef ARENA_ALLOCATOR_H
#define ARENA_ALLOCATOR_H
#include "allocator.h"
#include <errno.h>
#include <stdint.h>
#include <string.h>

OwnAllocator arena_owned_new(void);
My_allocator *arena_new(size_t blockSize);
My_allocator *arena_new_ext(const My_allocator *base, size_t blockSize);
void arena_cleanup(My_allocator *arena);
size_t arena_footprint(My_allocator *arena);
static void arena_cleanup_handler(My_allocator **arenaPtr) {
  if (arenaPtr && *arenaPtr) {
    arena_cleanup(*arenaPtr);
    *arenaPtr = NULL;
  }
}
#define Arena_scoped [[gnu::cleanup(arena_cleanup_handler)]] My_allocator
#endif // ARENA_ALLOCATOR_H
#ifdef ARENA_ALLOCATOR_C
typedef struct ArenaBlock ArenaBlock;
typedef struct ArenaBlock {
  ArenaBlock *next;
  size_t place;
  size_t size;
  size_t *freelist[10];
  uint8_t *buffer;
  const My_allocator *allocator;
} ArenaBlock;
void arenablock_free(ArenaBlock *block);
void arena_free(const My_allocator *allocator, void *ptr);
void *arena_alloc(const My_allocator *ref, size_t size);
void *arena_r_alloc(const My_allocator *arena, void *ptr, size_t size);
ArenaBlock *arenablock_new(const My_allocator *allocator, size_t blockSize);

void arena_cleanup(My_allocator *arena) {
  ArenaBlock *it = (ArenaBlock *)(arena->arb);
  const My_allocator *allocator = it->allocator;
  arenablock_free(it);
  aFree(allocator, arena);
}
void arenablock_free(ArenaBlock *block) {
  while (block) {
    ArenaBlock *next = block->next;
    const My_allocator *allocator = block->allocator;
    aFree(allocator, block);
    block = next;
  }
}
size_t arena_footprint(My_allocator *arena) {
  size_t res = 0;
  ArenaBlock *block = (ArenaBlock *)(arena->arb);
  while (block) {
    ArenaBlock *next = block->next;
    res += block->size;
    block = next;
  }
  return res;
}
void arena_free(const My_allocator *allocator, void *ptr) {
  size_t *thisSize = (size_t *)((uint8_t *)ptr - sizeof(size_t));
  ArenaBlock *it = (ArenaBlock *)(allocator->arb);
  for (int i = 0; i < 10; i++) {
    if (!it->freelist[i]) {
      it->freelist[i] = thisSize;
      return;
    }
  }
  for (int i = 0; i < 10; i++) {
    if (*it->freelist[i] < *thisSize) {
      it->freelist[i] = thisSize;
      return;
    }
  }
  // noop
}

My_allocator *ownArenaInit(void) {
  return arena_new(1024);
}
void ownArenaDeInit(My_allocator *d) {
  return arena_cleanup(d);
}
OwnAllocator arena_owned_new(void) {
  return (OwnAllocator){ownArenaInit, ownArenaDeInit};
}
My_allocator *arena_new_ext(const My_allocator *base, size_t blockSize) {
  My_allocator *res =
      (My_allocator *)aAlloc(base, sizeof(My_allocator));
  if (!res)
    exit(ENOMEM);
  *res =
      (My_allocator){
          arena_alloc,
          arena_free,
          arena_r_alloc,
      };
  res->arb = arenablock_new(base, blockSize);
  return res;
}
My_allocator *arena_new(size_t blockSize) {
  return arena_new_ext(defaultAlloc, blockSize);
}
void *arena_r_alloc(const My_allocator *arena, void *ptr, size_t size) {
  size = (size + alignof(max_align_t) - 1) / alignof(max_align_t) * alignof(max_align_t);
  if (!ptr)
    exit(1);
  size_t *lastSize = (size_t *)((uint8_t *)ptr - sizeof(size_t));
  if (*lastSize > size)
    return ptr;
  void *res = arena_alloc(arena, size);
  memmove(res, ptr, (*lastSize));
  arena_free(arena, ptr);
  return res;
}
void *arena_alloc(const My_allocator *ref, size_t size) {
  size = (size + alignof(max_align_t) - 1) / alignof(max_align_t) * alignof(max_align_t);
  ArenaBlock *it = (ArenaBlock *)(ref->arb);
  void *res = NULL;
  for (int i = 0; i < 10; i++) {
    if (it->freelist[i]) {
      size_t *sizeptr = it->freelist[i];
      if (*sizeptr >= size) {
        it->freelist[i] = NULL;
        return (void *)(sizeptr + 1);
      }
    }
  }
  while (!res) {
    if (it->size - it->place >= size + sizeof(size_t)) {
      *(size_t *)(it->buffer + it->place) = size;
      it->place += sizeof(size_t);

      res = it->buffer + it->place;
      it->place += size;
    } else {
      if (!it->next) {
        int minsize = size + sizeof(size_t);
        minsize = (minsize < it->size ? it->size : minsize);
        it->next = arenablock_new(it->allocator, minsize);
      }
      it = it->next;
    }
  }
  return res;
}
ArenaBlock *arenablock_new(const My_allocator *allocator, size_t blockSize) {
  ArenaBlock *res = (ArenaBlock *)aAlloc(allocator, (sizeof(ArenaBlock) + blockSize));
  if (!res)
    exit(ENOMEM);
  *res = (ArenaBlock){
      .next = NULL,
      .place = 0,
      .size = blockSize,
      .freelist =
          {
              NULL,
              NULL,
              NULL,
              NULL,
              NULL,
              NULL,
              NULL,
              NULL,
              NULL,
              NULL,
          },
      .buffer = (uint8_t *)res + sizeof(ArenaBlock),
      .allocator = allocator,
  };
  return res;
}
#endif // ARENA_ALLOCATOR_C
