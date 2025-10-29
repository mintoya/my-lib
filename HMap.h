#ifndef HMAP_H
#define HMAP_H
#include "um_fp.h"
typedef struct {
} HMap;
unsigned int HMap_hash(um_fp);
#endif // HMAP_H

#ifdef HMAP_C
unsigned int HMap_hash(um_fp str) {
  unsigned long h = 5381;
  for (; str.width > 0; str.width--)
    h = ((h << 5) + h) + (str.ptr)[str.width - 1];
  return h;
}
#endif
