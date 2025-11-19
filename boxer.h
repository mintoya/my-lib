#ifndef BOXER_H
#define BOXER_H
#include "allocator.h"
#include "fptr.h"
typedef enum {
  KNOWN, // size known
  FPTR,  // size embedded
} boxertype;
typedef struct {
  boxertype type;
  size_t size;
} boxer;
typedef struct {
  unsigned int ammount;
  boxer boxers[];
} Boxer;

inline static fptr enBox(const My_allocator *allocator, const Boxer *const Boxer, void **ptrs) {
  boxer boxes[Boxer->ammount];
  for (int i = 0; i < Boxer->ammount; i++) {
    boxes[i] = Boxer->boxers[i];
  }
  size_t totalsize = 0;
  for (size_t i = 0; i < Boxer->ammount; i++) {
    switch (boxes[i].type) {
    case KNOWN: {
      totalsize += boxes[i].size;
    } break;
    case FPTR: {
      fptr p = (*(fptr *)(ptrs[i]));
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
  for (size_t i = 0; i < Boxer->ammount; i++) {
    switch (boxes[i].type) {
    case KNOWN: {
      memcpy(res.ptr + place, ptrs[i], boxes[i].size);
    } break;
    case FPTR: {
      fptr p = (*(fptr *)(ptrs[i]));
      size_t thisSize = p.width;
      memcpy(res.ptr + place, &thisSize, sizeof(size_t));
      memcpy(res.ptr + place + sizeof(size_t), p.ptr, thisSize);
    } break;
    }
    place += boxes[i].size;
  }
  return res;
}
inline static void unBox(const Boxer *const Boxer, void **ptrs, fptr boxed) {
  boxer boxes[Boxer->ammount];
  for (int i = 0; i < Boxer->ammount; i++) {
    boxes[i] = Boxer->boxers[i];
  }
  size_t place = 0;
  for (size_t i = 0; i < Boxer->ammount; i++) {
    switch (boxes[i].type) {
    case KNOWN:
      memcpy(ptrs[i], boxed.ptr + place, boxes[i].size);
      place += boxes[i].size;
      break;
    case FPTR: {
      size_t tsize = (*(size_t *)(boxed.ptr + place));
      boxes[i].size = sizeof(size_t) + tsize;
      bFptr **p = (bFptr **)ptrs[i];
      *p = (bFptr *)(boxed.ptr + place);
      place += boxes[i].size;
    } break;
    }
  }
}

#endif // BOXER_H
