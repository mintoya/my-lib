#include "umap-printer/printer.h"
#include "umap/umap.h"
#include <alloca.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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

  for (int i = 0; i < string.length; i++) {
    if (((char *)string.ptr)[i] == front) {
      unsigned int counter = 1;
      for (int ii = 1; ii + i < string.length; ii++) {
        if (((char *)string.ptr)[i + ii] == front) {
          counter++;
        } else if (((char *)string.ptr)[i + ii] == back) {
          counter--;
        }
        if (!counter)
          return (um_fp){.ptr = ((char *)string.ptr) + i + 1, .length = ii - 1};
      }
    }
  }
  return nullUmf;
}
um_fp around(char limits[2], um_fp string) {

  char front = limits[0];
  char back = limits[1];

  for (int i = 0; i < string.length; i++) {
    if (((char *)string.ptr)[i] == front) {
      unsigned int counter = 1;
      for (int ii = 1; ii + i < string.length; ii++) {
        if (((char *)string.ptr)[i + ii] == front) {
          counter++;
        } else if (((char *)string.ptr)[i + ii] == back) {
          counter--;
        }
        if (!counter)
          return (um_fp){.ptr = ((char *)string.ptr) + i, .length = ii + 1};
      }
    }
  }
  return nullUmf;
}

um_fp until(char delim, um_fp string) {
  int i = 0;
  while (i < string.length && ((char *)string.ptr)[i] != delim) {
    i++;
  }
  string.length = i;
  return string;
}
um_fp behind(char delim, um_fp string) {
  int i = 0;
  while (i < string.length && ((char *)string.ptr)[i] != delim) {
    i++;
  }
  string.length = i + 1;
  return string;
}

#define stack_split(result, string, ...)                                       \
  result = alloca(                                                             \
      sizeof(um_fp) *                                                          \
      (sizeof((unsigned int[]){__VA_ARGS__}) / sizeof(unsigned int) + 1));     \
  do {                                                                         \
    void *last;                                                                \
    unsigned int args[] = {__VA_ARGS__};                                       \
    for (int i = 0; i < sizeof(args) / sizeof(unsigned int); i++) {            \
      result[i] = (um_fp){                                                     \
          .ptr = (i == 0) ? (string.ptr) : (last),                             \
          .length = (i == 0) ? (args[0]) : (args[i] - args[i - 1]),            \
      };                                                                       \
      assert((i == 0) ? (args[0]) : (args[i] - args[i - 1]) > 0);              \
      last = ((char *)result[i].ptr) + result[i].length;                       \
    }                                                                          \
    result[sizeof(args) / sizeof(unsigned int)] =                              \
        (um_fp){.ptr = last, .length = string.length - (last - string.ptr)};   \
  } while (0);

void parse_word(um_fp string) {
  usePrintln(char *, __func__);
  usePrintln(um_fp, string);
}
void parse_list(um_fp string) {
  usePrintln(char *, __func__);
  usePrintln(um_fp, string);
}
void parse_object(um_fp string) {
  usePrintln(char *, __func__);
  usePrintln(um_fp, string);
}

void *parser(um_fp string, UMap *parent) {
  if (!(string.ptr && string.length)) {
    return NULL;
  }
  um_fp name = until(':', string);
  usePrintln(um_fp, name);

  int place = behind(':', string).length;
  while (((char *)(string.ptr))[place] && ((char *)(string.ptr))[place] == ' ')
    place++;

  char current = ((char *)(string.ptr))[place];
  um_fp toParse;

  while (string.length) {
    if (current == '{') {
      um_fp toParse = inside("{}", string);
      parse_object(toParse);
    } else if (current == '[') {
      um_fp toParse = inside("{}", string);
      parse_list(toParse);
    } else {
      um_fp toParse = inside(":;", string);
      parse_word(toParse);
    }
    string = nullUmf;
  }

  // while ()
  //   name = behind(':', string);
}

char str[] = {
#embed "test.txt"
};

int main(void) {

  char *testString = str;
  // um_fp str = um_from(testString);
  um_fp str = um_from("Mozilla");
  um_fp *slices = stack_split(slices, str, 1, 3);

  usePrintln(um_fp, slices[1]);

  slices = stack_split(slices, str, 2);

  usePrintln(um_fp, slices[1]);

  // usePrint(um_fp, slices[2]);
  // usePrintln(um_fp,around("{}",str));
  // usePrintln(um_fp,inside("{}",str));
  // parser(str, NULL);
  return 0;
}
