#include "../my-list/my-list.h"
#include "../string-List/um_fp.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define START_BIT 0x67
#define END_BIT 0x41

#define CHECK_TYPE uint64_t
static inline CHECK_TYPE cSum_CHECK_EXPR(CHECK_TYPE val, uint8_t byte) {
  uint32_t sum = val & 0x0000ffff;
  uint32_t x0r = (val & 0xffff0000) >> 16;

  sum += byte;
  x0r ^= byte;

  return ((uint32_t)x0r << 16) | sum;
}

// #define CHECK_TYPE uint8_t
// #define cSum_CHECK_EXPR(val, byte) val = val^byte

// #define CHECK_TYPE uint32_t
// #define cSum_CHECK_EXPR(val, byte) val = ((val + (byte)) << 1) ^ (val ^ (byte))

// start_bit .. data  .. sum .. end_bit
typedef struct {
  List *checkSumScratch;
} dataChecker;

typedef struct {
  um_fp data;
} checkData;
static dataChecker cSum_new() {
  return (dataChecker){.checkSumScratch = List_new(sizeof(char))};
}
static void cSum_free(dataChecker a) { List_free(a.checkSumScratch); }
static checkData cSum_toSum(dataChecker d, um_fp data) {
  CHECK_TYPE sum = 0;
  for (unsigned int i = 0; i < data.width; i++)
    sum = cSum_CHECK_EXPR(sum, ((char *)data.ptr)[i]);

  d.checkSumScratch->length = 0;
  unsigned int newWidth = data.width + sizeof(char) * 2 + sizeof(CHECK_TYPE);
  if (newWidth > d.checkSumScratch->width)
    List_resize(d.checkSumScratch, newWidth);
  mList_add(d.checkSumScratch, char, START_BIT);
  List_appendFromArr(d.checkSumScratch, data.ptr, data.width);
  List_appendFromArr(d.checkSumScratch, &sum, sizeof(CHECK_TYPE));
  mList_add(d.checkSumScratch, char, END_BIT);
  checkData res = {.data = {
                       .ptr = d.checkSumScratch->head,
                       .width = d.checkSumScratch->length,
                   }};
  return res;
}
static um_fp cSum_fromSum(checkData data) {
  char *ptr = (char *)data.data.ptr;
  if (data.data.width < sizeof(char) * 2 + sizeof(CHECK_TYPE)) {
    // fprintf(stderr, "message too short to contain badding!");
    return nullUmf;
  }
  if (ptr[0] != START_BIT || ptr[data.data.width - 1] != END_BIT) {
    // fprintf(stderr, "start/end bit not found ");
    return nullUmf;
  }
  CHECK_TYPE sum = 0;
  for (unsigned int i = 1; i < data.data.width - 1 - (sizeof(CHECK_TYPE)); i++)
    sum = cSum_CHECK_EXPR(sum, ptr[i]);
  if (sum !=
      *(CHECK_TYPE *)(ptr + (data.data.width - 1 - (sizeof(CHECK_TYPE))))) {
    // fprintf(stderr, "checksum is false");
    return nullUmf;
  }
  return (um_fp){.ptr = ptr + 1,
                 .width = data.data.width - 2 - sizeof(CHECK_TYPE)};
}
static CHECK_TYPE cSum_getCheck(checkData cd) {
  char *ptr = (char *)cd.data.ptr;
  return ptr[cd.data.width - 2];
}
#undef START_BIT
#undef END_BIT
#undef CHECK_EXPR
#undef CHECK_TYPE
