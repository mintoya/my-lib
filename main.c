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

um_fp removeSpacesPadding(um_fp in) {
  um_fp res = in;
  int front = 0;
  int back = in.width - 1;
  while (front < in.width && ((char *)in.ptr)[front] == ' ') {
    front++;
  }
  while (back > front && ((char *)in.ptr)[back] == ' ') {
    back--;
  }
  um_fp *splits = stack_split(splits, in, front, back);
  res = splits[1];
  return res;
}
um_fp parseNext(um_fp string, UMap parent[static 1]) {
  if (!(string.ptr && string.width)) {
    return nullUmf;
  }

  um_fp name = until(':', string);
  if (name.ptr == string.ptr && name.width == string.width) {
    return nullUmf;
  }

  um_fp next = after(string, behind(':', string));

  next = removeSpacesPadding(next);

  if (!(next.ptr && next.width)) {
    return nullUmf;
  }

  um_fp toParse = string ;

  switch (((char *)next.ptr)[0]) {
  case '[':
    toParse = around("[]", next);
    UMap_set(parent, name, um_from("list here!"));
    break;
  case '{':
    toParse = around("{}", next);
    UMap_set(parent, name, um_from("object here!"));
    break;
  default:
    toParse = behind(';', next);
    UMap_set(parent, name, until(';', next));
    break;
  }
  return toParse;
}

// char inputString[] = {
// #embed "test.txt"
// };

char* inputString = 
R"d(

x  : y ;
x2 :{ x2 : y2; } 
x3 :[ y3 , y4 ] 

)d";


void stringList_printMeta(const stringList *sl) {
  if (!sl) {
    printf("stringList (null)\n");
    return;
  }

  List *metaList = &sl->List_stringMetaData;
  printf("stringList metadata (length=%u):\n", metaList->length);

  for (unsigned int i = 0; i < metaList->length; i++) {
    stringMetaData *meta = (stringMetaData *)List_gst(metaList, i);
    if (!meta) {
      printf("  [%u] <null>\n", i);
      continue;
    }
    printf("  [%u] index=%u, width=%u, size=%u\n", i, meta->index, meta->width,
           meta->_size);
  }
}

int main(void) {
  printf(inputString);
  um_fp str = um_from(inputString);

  UMap *parent = UMap_new();

  while (str.ptr) {
    um_fp read = parseNext(str, parent);
    if (!(read.ptr && read.width)) {
      break;
    }
    str = after(str, read);
  }

  mPrint("UMap<string>", &parent);
  // usePrint(UMap *, parent);
  return 0;
}
