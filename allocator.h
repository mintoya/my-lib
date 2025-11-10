#ifndef MY_ALLOCATOR_H
#define MY_ALLOCATOR_H
#include <stddef.h>
typedef struct {
  void *(*alloc)(size_t);
  void (*free)(void *);
  void *(*r_alloc)(void *, size_t);
} My_allocator;

#include <stdlib.h>
static const My_allocator defaultAllocator = {
    .alloc = malloc,
    .free = free,
    .r_alloc = realloc,
};
#endif // MY_ALLOCATOR_H
