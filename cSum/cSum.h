#include "../my-list/my-list.h"
#include "../string-List/um_fp.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define START_BIT ((char)0x67)
#define END_BIT ((char)0x41)

#define CHECK_TYPE uint32_t

static inline uint32_t cSum_CHECK_EXPR(uint32_t crc, uint8_t data) {
    crc ^= (uint32_t)data << 24;

    for (int i = 0; i < 8; i++) {
        if (crc & 0x80000000u) {
            crc = (crc << 1) ^ 0x10210000u;  // polynomial aligned to top
        } else {
            crc <<= 1;
        }
    }

    return crc;
}


// #define CHECK_TYPE uint32_t
// static inline CHECK_TYPE cSum_CHECK_EXPR(CHECK_TYPE val,uint8_t data){
//   uint16_t top = ( val&0xffff0000 )>>16;
//   top+=data;
//   uint16_t bot = val&0x0000ffff;
//   bot^=data;
//   bot^=top;
//   return ( (uint32_t)top )<<16 | bot;
// }

// start_bit .. data .. data .. checksum .. end_bit
typedef struct {
  List *checkSumScratch;
} dataChecker;

typedef struct {
  um_fp data;
} checkData;

static dataChecker cSum_new() {
  List *l = mList(char);
  List_resize(l, 20);
  return (dataChecker){.checkSumScratch = l};
}
static void cSum_free(dataChecker a) { List_free(a.checkSumScratch); }

static checkData cSum_toSum(dataChecker d, um_fp data) {
  CHECK_TYPE sum = 0;
  for (unsigned int i = 0; i < data.width; i++) {
    sum = cSum_CHECK_EXPR(sum, ((char *)data.ptr)[i]);
  }
  d.checkSumScratch->length = 0;
  unsigned int newWidth = data.width*2 + sizeof(char) * 2 + sizeof(CHECK_TYPE);
  if (newWidth > d.checkSumScratch->width)
    List_resize(d.checkSumScratch, newWidth);

  mList_add(d.checkSumScratch, char, START_BIT);
  List_appendFromArr(d.checkSumScratch, data.ptr, data.width);
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
  char* ptr1,*ptr2;
  CHECK_TYPE check;
  CHECK_TYPE checkResult = 0;
  ptr1 = (char*)data.data.ptr;
  if(ptr1[0]!=START_BIT){
    return nullUmf;
  }
  if (ptr1[data.data.width-1]!=END_BIT) {
    return nullUmf;
  }
  unsigned int dataLength = data.data.width;
  dataLength -= (sizeof(char)+sizeof(CHECK_TYPE));

  check = *(CHECK_TYPE*)(data.data.ptr+dataLength);

  dataLength -= sizeof(char);
  dataLength/=2;

  ptr2 = ptr1 = ptr1+sizeof(char);
  ptr2 +=dataLength;

  for (unsigned int  i = 0; i < dataLength ; i++) {
    if(ptr1[i]!=ptr2[i]){
      return nullUmf;
    } 
    checkResult = cSum_CHECK_EXPR(checkResult,ptr1[i]);
  }
  if(checkResult!=check){
    return nullUmf;
  }
  return (um_fp){.ptr = ptr1,.width = dataLength};
}
#undef START_BIT
#undef END_BIT
#undef CHECK_EXPR
#undef CHECK_TYPE
