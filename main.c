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
// index after end if c not present
unsigned int um_indexOf(um_fp string, char c) {
  int i;
  for (i = 0; i < string.width; i++) {
    if (((char *)string.ptr)[i] == c) {
      return i;
    }
  }
  return i;
}
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
        if (i + ii == string.width - 1)
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
        if (!counter)
          return (um_fp){.ptr = ((char *)string.ptr) + i, .width = ii + 1};
        if (i + ii == string.width - 1)
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
  if (!(main.ptr && main.width))
    return nullUmf;
  if (!(slice.ptr && slice.width))
    return main;
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
char isExtrenuous(char c) {
  switch (c) {
  case ' ':
    return 1;
  case '\n':
    return 1;
  case '\0':
    return 1;
  case '\t':
    return 1;
  default:
    return 0;
  }
}
um_fp removeSpacesPadding(um_fp in) {
  um_fp res = in;
  int front = 0;
  int back = in.width - 1;
  while (front < in.width && isExtrenuous(((char *)in.ptr)[front])) {
    front++;
  }
  while (back > front && isExtrenuous(((char *)in.ptr)[back])) {
    back--;
  }
  back++;
  um_fp *splits = stack_split(splits, in, front, back);
  res = splits[1];
  return res;
}
typedef struct {
  um_fp key;
  um_fp value;
  um_fp footprint;
} kVf;
kVf parseNext(um_fp string) {
  if (!(string.ptr && string.width)) {
    return (kVf){nullUmf, nullUmf};
  }

  um_fp name = until(':', string);
  name = removeSpacesPadding(name);
  if (name.ptr == string.ptr && name.width == string.width) {
    return (kVf){nullUmf, nullUmf};
  }

  um_fp next = after(string, behind(':', string));

  next = removeSpacesPadding(next);

  if (!(next.ptr && next.width)) {
    return (kVf){nullUmf, nullUmf};
  }

  um_fp toParse;
  um_fp value;

  switch (((char *)next.ptr)[0]) {
  case '[':
    toParse = around("[]", next);
    value = removeSpacesPadding(inside("[]", next));
    break;
  case '{':
    toParse = around("{}", next);
    value = removeSpacesPadding(inside("{}", next));
    break;
  default:
    toParse = behind(';', next);
    value = removeSpacesPadding(until(';', next));
    break;
  }
  return (kVf){name, value, toParse};
}
um_fp findIndex(um_fp str, unsigned int index) {
  str = removeSpacesPadding(str);
  um_fp thisValue;
  char isobj = 0;
  switch (*(char *)str.ptr) {
  case ('['):
    thisValue = around("[]", str);
    isobj = 1;
    break;
  case ('{'):
    thisValue = around("{}", str);
    isobj = 2;
    break;
  default:
    thisValue = until(',', str);
  }
  if (!index) {
    if (isobj) {
      if (isobj == 1)
        return inside("[]", thisValue);
      if (isobj == 2)
        return inside("{}", thisValue);
    }
    return thisValue;
  } else {
    um_fp next = after(str, thisValue);
    if (um_indexOf(next, ',') < next.width) {
      next = after(str, behind(',', next));
      return findIndex(next, index - 1);
    } else {
      return nullUmf;
    }
  }
}
um_fp findKey(um_fp str, um_fp key) {
  key = removeSpacesPadding(key);
  while (str.ptr) {
    kVf read = parseNext(str);
    if (!(read.key.ptr && read.key.width)) {
      // usePrintln(char * ,"null");
      return nullUmf;
    } else if (um_eq(key, read.key)) {
      return read.value;
    }
    str = after(str, read.footprint);
  }
  return nullUmf;
}

typedef struct {
  enum {
    NONE = 0,
    STRING = 1,
    INDEX = 2,
  } type;
  union {
    um_fp pArg;
    unsigned int index;
  } data;
} parseArg;

um_fp findAny(um_fp str, parseArg ki) {
  switch (ki.type) {
  case STRING:
    return findKey(str, ki.data.pArg);
    break;
  case INDEX:
    return findIndex(str, ki.data.index);
    break;
  default:
    return nullUmf;
  }
}
#define fIndex(val) ((parseArg){.type = INDEX, .data.index = val})
#define fKey(val) ((parseArg){.type = STRING, .data.pArg = um_from(val)})

um_fp find_many(um_fp str, ...) {
  va_list ap;
  va_start(ap, str);
  parseArg key;
  while ((key = va_arg(ap, parseArg)).type) {
    if (key.type == STRING)
      str = findKey(str, key.data.pArg);
    if (key.type == INDEX)
      str = findIndex(str, key.data.index);
  }

  va_end(ap);
  return str;
}
#define find(um, ...) find_many(um, __VA_ARGS__, ((parseArg){.type = NONE}))

char *inputString =
R"d(

layer0: [
    KEY_TAB, C('q'), C('w'), C('e'), C('r'), C('t'),        C('y'), C('u'), C('i'), C('o'), C('p'),  KEY_BACKSPACE,
    M('s') , C('a'), C('s'), C('d'), C('f'), C('g'),        C('h'), C('j'), C('k'), C('l'), C(';'),  KEY_LEFTBRACE,
    M('c') , C('z'), C('x'), C('c'), C('v'), C('b'),        C('n'), C('m'), C(','), C('.'), C('/'),  KEY_ENTER,
    0      , 0     , 0     , M('a'), C(' '),  TD(0),        L(1)  ,   L(2),      0,      0,      0,  0,
]
layer1: [
    KEY_ESC, C('1'), C('2'), C('3'), C('4'), C('5'),        C('6'), C('7'), C('8'), C('9'), C('0'),  KEY_DELETE,
    0      , 0     , 0     , 0     , 0     , 0     ,        0     , 0     , 0     , 0     , 0     ,  0,
    0      , 0     , 0     , 0     , 0     , 0     ,        C('='),C('-') , 0     , 0     , 0     ,  0,
    0      , 0     , 0     , 0     , 0     , 0     ,        0     , 0     , 0     , 0     , 0     ,  0,
]
layer2:[
    0      , 0     , 0     , 0     , 0     , 0     ,        0     , 0     , D('W'), 0     , 0     ,  0,
    0      , 0     , 0     , 0     , 0     , 0     ,        0     , D('A'), D('S'), D('D'), 0     ,  0,
    0      , 0     , 0     , 0     , 0     , 0     ,        0     , 0     , 0     , 0     , 0     ,  0,
    0      , 0     , 0     , 0     , 0     , 0     ,        0     , 0     , 0     , 0     , 0     ,  0,
]

)d";

int main(void) {
  // printf(inputString);
  um_fp str = um_from(inputString);

  usePrintln(um_fp,find(str,fKey("layer0"),fIndex(30)));

  return 0;
}
