#include "string-List/stringList.h"
#include "umap-printer/printer.h"
#include "umap/umap.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <assert.h>
#include <malloc.h>
#include <windows.h>
#else
#include <alloca.h>
#endif

#define min(a, b) ((a < b) ? (a) : (b))
#define max(a, b) ((a > b) ? (a) : (b))

um_fp inside(char limits[2], um_fp string) {
  char front = limits[0];
  char back = limits[1];

  for (int i = 0; i < string.width; i++) {
    if (((char *)string.ptr)[i] == front) {
      unsigned int counter = 1;
      for (int ii = 1; ii + i < string.width; ii++) {
        if (((char *)string.ptr)[i + ii] == front) {
          counter++;
        } else if (((char *)string.ptr)[i + ii] == back) {
          counter--;
        }
        if (!counter)
          return (um_fp){.ptr = ((char *)string.ptr) + i + 1, .width = ii - 1};
        if(i+ii==string.width-1)
          return (um_fp){.ptr = ((char *)string.ptr) + i + 1, .width = ii - 1};
      }
    }
  }
  return nullUmf;
}
um_fp around(char limits[2], um_fp string) {

  char front = limits[0];
  char back = limits[1];

  for (int i = 0; i < string.width; i++) {
    if (((char *)string.ptr)[i] == front) {
      unsigned int counter = 1;
      for (int ii = 1; ii + i < string.width; ii++) {
        if (((char *)string.ptr)[i + ii] == front) {
          counter++;
        } else if (((char *)string.ptr)[i + ii] == back) {
          counter--;
        }
        if (!counter )
          return (um_fp){.ptr = ((char *)string.ptr) + i, .width = ii + 1};
        if(i+ii==string.width-1)
          return (um_fp){.ptr = ((char *)string.ptr) + i, .width = ii + 1};
      }
    }
  }
  return nullUmf;
}

um_fp until(char delim, um_fp string) {
  int i = 0;
  while (i < string.width && ((char *)string.ptr)[i] != delim) {
    i++;
  }
  string.width = i;
  return string;
}
um_fp behind(char delim, um_fp string) {
  int i = 0;
  while (i < string.width && ((char *)string.ptr)[i] != delim) {
    i++;
  }
  string.width = min(i + 1, string.width);
  return string;
}
um_fp after(um_fp main, um_fp slice) {
  if(!(main.ptr&&main.width)) return nullUmf;
  if(!(slice.ptr&&slice.width)) return main;
  char *mainStart = main.ptr;
  char *mainEnd = mainStart + main.width;
  char *sliceStart = slice.ptr;
  char *sliceEnd = sliceStart + slice.width;

  assert(sliceStart >= mainStart && sliceEnd <= mainEnd);

  return (um_fp){.ptr = sliceEnd, .width = mainEnd - sliceEnd};
}
#define stack_split(result, string, ...)                                       \
  result = alloca(                                                             \
      sizeof(um_fp) *                                                          \
      (sizeof((unsigned int[]){__VA_ARGS__}) / sizeof(unsigned int) + 1));     \
  do {                                                                         \
    void *last;                                                                \
    unsigned int args[] = {__VA_ARGS__};                                       \
    for (int i = 0; i < sizeof(args) / sizeof(unsigned int); i++) {            \
      args[i] = (i == 0) ? (min(args[i], string.width))                        \
                         : (min(string.width, max(args[i], args[i - 1])));     \
      result[i] = (um_fp){                                                     \
          .ptr = (i == 0) ? (string.ptr) : (last),                             \
          .width = (i == 0) ? (args[0]) : (args[i] - args[i - 1]),             \
      };                                                                       \
      last = ((char *)result[i].ptr) + result[i].width;                        \
    }                                                                          \
    result[sizeof(args) / sizeof(unsigned int)] =                              \
        (um_fp){.ptr = last, .width = string.width - (last - string.ptr)};     \
  } while (0);
char isExtrenuous(char c){
  switch (c) {
    case ' ':return 1;
    case '\n':return 1;
    case '\0':return 1;
    case '\t':return 1;
    default:
      return 0;
  }
}
um_fp removeSpacesPadding(um_fp in) {
  um_fp res = in;
  int front = 0;
  int back = in.width - 1;
  while (front < in.width &&  isExtrenuous(((char *)in.ptr)[front]) ) {
    front++;
  }
  while (back > front && isExtrenuous(((char *)in.ptr)[back] )) {
    back--;
  }
  back++;
  um_fp *splits = stack_split(splits, in, front, back);
  res = splits[1];
  return res;
}
typedef struct {um_fp key; um_fp value;um_fp footprint;}kVf;
kVf parseNext(um_fp string){
  if (!(string.ptr && string.width)) {
    return (kVf){nullUmf,nullUmf};
  }

  um_fp name = until(':', string);
  name = removeSpacesPadding(name);
  if (name.ptr == string.ptr && name.width == string.width) {
    return (kVf){nullUmf,nullUmf};
  }

  um_fp next = after(string, behind(':', string));

  next = removeSpacesPadding(next);

  if (!(next.ptr && next.width)) {
    return (kVf){nullUmf,nullUmf};
  }

  um_fp toParse;
  um_fp value;

  switch (((char *)next.ptr)[0]) {
  case '[':
    toParse = around("[]", next);
    value =removeSpacesPadding(inside("[]", next));
    break;
  case '{':
    toParse = around("{}", next);
    value =removeSpacesPadding(inside("{}", next));
    break;
  default:
    toParse = behind(';', next);
    value = removeSpacesPadding(until(';', next));
    break;
  }
  return (kVf){name,value,toParse};
}
um_fp findIndex(um_fp str, unsigned int index){
  str = removeSpacesPadding(str);
  um_fp thisValue;
  char isobj = 0;
  switch(*(char*)str.ptr){
    case('['):thisValue = around("[]",str);isobj = 1;break;
    case('{'):thisValue = around("{}",str);isobj = 2;break;
    default: thisValue = until(',',str);
  }
  if(!index){
    if(isobj){
      if(isobj==1)
        return inside("[]",thisValue);
      if(isobj==2)
        return inside("{}",thisValue);
    }
    return thisValue;
  }else{
    // thisValue.width++;
    // um_fp next = after(str,thisValue);
    // if(isobj){
    //   next = after(str,behind(',',next));
    // }
    um_fp next = after(str,thisValue);
    next=after(str,behind(',',next));
    return findIndex(next,index-1);
  }
}
um_fp find_Functoin(um_fp str,um_fp key){
  key = removeSpacesPadding(key);
  while (str.ptr) {
    kVf read = parseNext(str);
    if (!(read.key.ptr && read.key.width)) {
      // usePrintln(char * ,"null");
      return nullUmf;
    }else if(um_eq(key,read.key)){
      return read.value;
    }
    str = after(str, read.footprint);
  }
  return nullUmf;
}
um_fp find_many(um_fp str, ...) {
    va_list ap;
    va_start(ap, str);
    um_fp key;
    while (!um_eq(key = va_arg(ap, um_fp),nullUmf)) {
        str = find_Functoin(str, key);
    }

    va_end(ap);
    return str;
}
#define find(um,...) find_many(um,__VA_ARGS__,nullUmf)



char* inputString = 
R"d(
x  : y ;
x2 :{ x2 : y2; } 
x3 : [ [i], [i0,i2,i3], { index:1; }, { index:2; }, ]
)d";


int main(void) {
  // printf(inputString);
  um_fp str = um_from(inputString);

  // usePrintln(um_fp, find(str,um_from("x")));
  // usePrintln(um_fp, find(str,um_from("x2")));
  usePrintln(um_fp, find(str,um_from("x2"),um_from("x2")));

  return 0;
}
