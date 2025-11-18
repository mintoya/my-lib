#include "fptr.h"
#include "allocator.h"
// #define deRef(type, ptr) (*(type *)(ptr))
#define deRef(type, ptr) ({          \
  type tmp;                          \
  memcpy(&tmp, (ptr), sizeof(type)); \
  tmp;                               \
})
typedef struct {
  enum {
    KNOWN, // size known
    FPTR,  // size embedded
  } type;
  size_t size;
} boxer;

fptr enBox(const My_allocator *allocator, size_t n, boxer boxes[n], void *ptrs[n]) {
  size_t totalsize = 0;
  for (size_t i = 0; i < n; i++) {
    switch (boxes[i].type) {
    case KNOWN: {
      totalsize += boxes[i].size;
    } break;
    case FPTR: {
      fptr p = deRef(fptr, ptrs[i]);
      totalsize += sizeof(size_t) + p.width;
      boxes[i].size = sizeof(size_t) + p.width;
    } break;
    }
  }
  fptr res = (fptr){
      .width = totalsize,
      .ptr = (uint8_t *)aAlloc(allocator, totalsize),
  };
  size_t place = 0;
  for (size_t i = 0; i < n; i++) {
    switch (boxes[i].type) {
    case KNOWN: {
      memcpy(res.ptr + place, ptrs[i], boxes[i].size);
    } break;
    case FPTR: {
      fptr p = deRef(fptr, ptrs[i]);
      size_t thisSize = p.width;
      memcpy(res.ptr + place, &thisSize, sizeof(size_t));
      memcpy(res.ptr + place + sizeof(size_t), p.ptr, thisSize);
    } break;
    }
    place += boxes[i].size;
  }
  return res;
}
void unBox(size_t n, boxer boxes[n], void *ptrs[n], fptr boxed) {
  size_t place = 0;
  for (size_t i = 0; i < n; i++) {
    switch (boxes[i].type) {
    case KNOWN:
      memcpy(ptrs[i], boxed.ptr + place, boxes[i].size);
      place += boxes[i].size;
      break;
    case FPTR: {
      size_t tsize = deRef(size_t, boxed.ptr + place);
      boxes[i].size = sizeof(size_t) + tsize;
      fptr *p = ptrs[i];
      p->ptr = boxed.ptr + place + sizeof(size_t);
      p->width = tsize;
      place += boxes[i].size;
    } break;
    }
  }
}
