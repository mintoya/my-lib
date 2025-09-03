#include "umap-printer/printer.h"
#include "umap/umap.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <malloc.h>
#else
#include <alloca.h>
#endif

char *delims = ":,{}[]; ";
char *sep = ",";
char *val = ":;";
char *lst = "[]";
char *obj = "{}";
// delimiters:
// : -> follows name
// ; -> ends defenition
// , -> separates lists
// { -> starts internal umap
// } -> ends internal umap
// [ -> starts internal list
// ] -> ends internal list
/*
  objects:
  name:<string/list/object>
  lists:
  name:[<string/list/object>,<string/list/object>...]
 */

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
  string.width = i + 1;
  return string;
}
#define min(a, b) ((a < b) ? (a) : (b))
#define max(a, b) ((a > b) ? (a) : (b))
#define stack_split(result, string, ...)                                       \
  result = alloca(                                                             \
      sizeof(um_fp) *                                                          \
      (sizeof((unsigned int[]){__VA_ARGS__}) / sizeof(unsigned int) + 1));     \
  do {                                                                         \
    void *last;                                                                \
    unsigned int args[] = {__VA_ARGS__};                                       \
    for (int i = 0; i < sizeof(args) / sizeof(unsigned int); i++) {            \
      args[i] = (i == 0) ? (min(args[i], string.width))                       \
                         : (min(string.width, max(args[i], args[i - 1])));    \
      result[i] = (um_fp){                                                     \
          .ptr = (i == 0) ? (string.ptr) : (last),                             \
          .width = (i == 0) ? (args[0]) : (args[i] - args[i - 1]),            \
      };                                                                       \
      last = ((char *)result[i].ptr) + result[i].width;                       \
    }                                                                          \
    result[sizeof(args) / sizeof(unsigned int)] =                              \
        (um_fp){.ptr = last, .width = string.width - (last - string.ptr)};   \
  } while (0);

void parse_word(um_fp string) {
  usePrintln(char *, __func__);
  usePrintln(um_fp, string);
}
void parse_list(um_fp string) {
  usePrintln(char *, __func__);
  usePrintln(um_fp, string);
}
void *parser(um_fp str, UMap *parent);
void parse_object(um_fp string) {
  usePrintln(char *, __func__);
  usePrintln(um_fp, string);
  parser(inside(obj, string), NULL);
}

void *parser(um_fp string, UMap *parent) {
  if (!(string.ptr && string.width)) {
    return NULL;
  }
  um_fp name = until(':', string);
  usePrintln(um_fp, name);

  int place = behind(':', string).width;
  while (((char *)(string.ptr))[place] && ((char *)(string.ptr))[place] == ' ')
    place++;

  char current = ((char *)(string.ptr))[place];
  um_fp toParse;

  while (string.width) {
    um_fp toParse;
    if (current == '{') {
      toParse = inside("{}", string);
      parse_object(toParse);
    } else if (current == '[') {
      toParse = inside("{}", string);
      parse_list(toParse);
    } else {
      toParse = inside(":;", string);
      parse_word(toParse);
    }
    unsigned int place = (toParse.ptr - string.ptr) + toParse.width + 1;
    um_fp *splits = stack_split(splits, string, place);
    string = splits[1];
    fflush(stdout);
  }

  return NULL;
}

// char str[] = {
// #embed "test.txt"
// };

int main(void) {
  // char *testString = str;
  // um_fp str = um_from(testString);

  // usePrint(um_fp, slices[2]);
  // usePrintln(um_fp,around("{}",str));
  // usePrintln(um_fp,inside("{}",str));
  // parser(str, NULL);

  UMap* n = UMap_new();
  UMap_set(n,um_from("hello"),um_from("world"));
  UMap_set(n,um_from("hellow"),um_from("world"));
  UMap_set(n,um_from("hello5"),um_from("world"));
  UMap_set(n,um_from("hello"),um_from("world"));
  usePrint(UMap *,n);
  return 0;
}
