#ifdef __cplusplus
#include "../string-List/um_fp.h"
extern "C" {
#endif
#ifndef KML_PARSER_H
#define KML_PARSER_H
#include <assert.h>
#include <malloc.h>
// #include <alloca.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define min(a, b) ((a < b) ? (a) : (b))
#define max(a, b) ((a > b) ? (a) : (b))

unsigned int kml_indexOf(um_fp string, char c);
um_fp kml_until(char delim, um_fp string);
um_fp kml_inside(char limits[2], um_fp string);
um_fp kml_around(char limits[2], um_fp string);
um_fp kml_behind(char delim, um_fp string);
um_fp kml_after(um_fp main, um_fp slice);
um_fp kml_removeSpacesPadding(um_fp in);
um_fp findIndex(um_fp str, unsigned int index);
um_fp findKey(um_fp str, um_fp key);
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

um_fp findAny(um_fp str, parseArg ki);

#ifndef __cplusplus
#define fIndex(val) ((parseArg){.type = INDEX, .data.index = val})
#define fKey(val) ((parseArg){.type = STRING, .data.pArg = um_from(val)})

// must be none terminated
um_fp find_many(um_fp str, ...);
#define find(um, ...) find_many(um, __VA_ARGS__, ((parseArg){.type = NONE}))
#else
static inline parseArg fIndex(unsigned int idx) {
  parseArg res;
  res.type = parseArg::INDEX;
  res.data.index = idx;
  return res;
}
static inline parseArg fKey(char *str) {
  parseArg res;
  res.type = parseArg::STRING;
  res.data.pArg = um_from(str);
  return res;
}
static const parseArg parseArg_terminator_v = {.type = parseArg::NONE};
#endif

#endif
#ifdef KML_PARSER_C

// index after end if c not present
unsigned int kml_indexOf(um_fp string, char c) {
  int i;
  char *ptr = (char *)string.ptr;
  for (i = 0; i < string.width && ptr[i] != c; i++)
    ;
  return i;
}
um_fp kml_inside(char limits[2], um_fp string) {
  char front = limits[0];
  char back = limits[1];

  int in_single = 0;
  int in_double = 0;

  for (int i = 0; i < string.width; i++) {
    char c = ((char *)string.ptr)[i];

    // toggle quote state
    if (c == '"' && !in_single)
      in_double = !in_double;
    else if (c == '\'' && !in_double)
      in_single = !in_single;

    if (!in_single && !in_double && c == front) {
      unsigned int counter = 1;
      for (int ii = 1; ii + i < string.width; ii++) {
        c = ((char *)string.ptr)[i + ii];

        // toggle quote state inside the nested scan
        if (c == '"' && !in_single)
          in_double = !in_double;
        else if (c == '\'' && !in_double)
          in_single = !in_single;

        if (!in_single && !in_double) {
          if (c == front) {
            counter++;
          } else if (c == back) {
            counter--;
          }
        }

        if (!counter)
          return (um_fp){.ptr = ((char *)string.ptr) + i + 1,
                         .width = (size_t)ii - 1};
        if (i + ii == string.width - 1)
          return (um_fp){.ptr = ((char *)string.ptr) + i + 1,
                         .width = (size_t)ii - 1};
      }
    }
  }
  return nullUmf;
}

um_fp kml_around(char limits[2], um_fp string) {
  char front = limits[0];
  char back = limits[1];

  int in_single = 0;
  int in_double = 0;

  for (int i = 0; i < string.width; i++) {
    char c = ((char *)string.ptr)[i];

    // toggle quote state
    if (c == '"' && !in_single)
      in_double = !in_double;
    else if (c == '\'' && !in_double)
      in_single = !in_single;

    if (!in_single && !in_double && c == front) {
      unsigned int counter = 1;
      for (int ii = 1; ii + i < string.width; ii++) {
        c = ((char *)string.ptr)[i + ii];

        if (c == '"' && !in_single)
          in_double = !in_double;
        else if (c == '\'' && !in_double)
          in_single = !in_single;

        if (!in_single && !in_double) {
          if (c == front) {
            counter++;
          } else if (c == back) {
            counter--;
          }
        }

        if (!counter)
          return (um_fp){.ptr = ((char *)string.ptr) + i,
                         .width = (size_t)ii + 1};
        if (i + ii == string.width - 1)
          return (um_fp){.ptr = ((char *)string.ptr) + i,
                         .width = (size_t)ii + 1};
      }
    }
  }
  return nullUmf;
}

um_fp kml_until(char delim, um_fp string) {
  int i = 0;
  char *ptr = (char *)string.ptr;
  while (i < string.width && ptr[i] != delim) {
    i++;
  }
  string.width = i;
  return string;
}
um_fp kml_behind(char delim, um_fp string) {
  int i = 0;
  while (i < string.width && ((char *)string.ptr)[i] != delim) {
    i++;
  }
  string.width = min(i + 1, string.width);
  return string;
}
um_fp kml_after(um_fp main, um_fp slice) {
  if (!(main.ptr && main.width))
    return nullUmf;
  if (!(slice.ptr && slice.width))
    return main;
  char *mainStart = (char *)main.ptr;
  char *mainEnd = mainStart + main.width;
  char *sliceStart = (char *)slice.ptr;
  char *sliceEnd = sliceStart + slice.width;

  assert(sliceStart >= mainStart && sliceEnd <= mainEnd);

  return (um_fp){.ptr = sliceEnd, .width = (size_t)(mainEnd - sliceEnd)};
}
#define stack_split(result, string, ...)                                       \
  result = (um_fp *)alloca(                                                    \
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
        (um_fp){.ptr = last,                                                   \
                .width = string.width - ((char *)last - (char *)string.ptr)};  \
  } while (0);
char isSkip(char c) {
  switch (c) {
  case ' ':
  // case '"':
  case '\n':
  case '\0':
  case '\t':
    return 1;
  default:
    return 0;
  }
}
um_fp kml_removeSpacesPadding(um_fp in) {
  um_fp res = in;
  int front = 0;
  int back = in.width - 1;
  while (front < in.width && isSkip(((char *)in.ptr)[front])) {
    front++;
  }
  while (back > front && isSkip(((char *)in.ptr)[back])) {
    back--;
  }
  um_fp *splits =
      stack_split(splits, in, (unsigned int)front, (unsigned int)back + 1);

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
  um_fp name = kml_until(':', string);
  name = kml_removeSpacesPadding(name);
  if (name.ptr == string.ptr && name.width == string.width) {
    return (kVf){nullUmf, nullUmf};
  }

  um_fp next = kml_after(string, kml_behind(':', string));

  next = kml_removeSpacesPadding(next);

  if (!(next.ptr && next.width)) {
    return (kVf){nullUmf, nullUmf};
  }

  um_fp toParse;
  um_fp value;

  switch (((char *)next.ptr)[0]) {
  case '[':
    toParse = kml_around("[]", next);
    value = kml_removeSpacesPadding(kml_inside("[]", next));
    break;
  case '{':
    toParse = kml_around("{}", next);
    value = kml_removeSpacesPadding(kml_inside("{}", next));
    break;
  case '"':
    next.ptr = (char *)next.ptr + 1;
    next.width--;

    value = kml_removeSpacesPadding(kml_until('"', next));
    toParse = kml_behind('"', next);

    toParse.ptr = (char *)toParse.ptr - 1;
    toParse.width++;

    next.ptr = (char *)next.ptr - 1;
    next.width++;
    break;
  default:
    toParse = kml_behind(';', next);
    value = kml_removeSpacesPadding(kml_until(';', next));
    break;
  }
  // if (name.width == 2 && ((char *)name.ptr)[1] == '/' &&
  //     ((char *)name.ptr)[0] == '/')
  //   usePrintln(char *, "key found a comment");
  return (kVf){name, value, toParse};
}
um_fp findIndex(um_fp str, unsigned int index) {
  if (!(str.ptr && str.width))
    return nullUmf;
  str = kml_removeSpacesPadding(str);
  um_fp thisValue;
  char isobj = 0, isComment = 0;
  switch (*(char *)str.ptr) {
  case ('['):
    thisValue = kml_around("[]", str);
    isobj = 1;
    break;
  case ('/'):
    isComment = 1;
  case ('{'):
    thisValue = kml_around("{}", str);
    isobj = 2;
    break;
  default:
    thisValue = kml_until(',', str);
  }
  if (isComment) {
    um_fp next = kml_after(str, thisValue);
    // usePrintln(char *, "index found a comment");
    return findIndex(next, index);
  }
  if (!index) {
    if (isobj) {
      if (isobj == 1)
        return kml_inside("[]", thisValue);
      if (isobj == 2)
        return kml_inside("{}", thisValue);
    }
    return thisValue;
  } else {
    um_fp next = kml_after(str, thisValue);
    if (kml_indexOf(next, ',') < next.width) {
      next = kml_after(str, kml_behind(',', next));
      return findIndex(next, index - 1);
    } else {
      return nullUmf;
    }
  }
}
um_fp findKey(um_fp str, um_fp key) {
  key = kml_removeSpacesPadding(key);
  while (str.ptr) {
    kVf read = parseNext(str);
    if (!(read.key.ptr && read.key.width)) {
      // usePrintln(char * ,"null");
      return nullUmf;
    } else if (um_eq(key, read.key)) {
      return read.value;
    }
    str = kml_after(str, read.footprint);
  }
  return nullUmf;
}

um_fp findAny(um_fp str, parseArg ki) {
#ifdef __cplusplus
  switch (ki.type) {
  case parseArg::STRING:
    return findKey(str, ki.data.pArg);
    break;
  case parseArg::INDEX:
    return findIndex(str, ki.data.index);
    break;
  default:
    return nullUmf;
  }
#else
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
#endif
}

um_fp find_many(um_fp str, ...) {
  va_list ap;
  va_start(ap, str);
  parseArg key;

#ifdef __cplusplus
  while ((key = va_arg(ap, parseArg)).type) {
    if (key.type == parseArg::STRING)
      str = findKey(str, key.data.pArg);
    if (key.type == parseArg::INDEX)
      str = findIndex(str, key.data.index);
  }
#else
  while ((key = va_arg(ap, parseArg)).type) {
    if (key.type == STRING)
      str = findKey(str, key.data.pArg);
    if (key.type == INDEX)
      str = findIndex(str, key.data.index);
  }
#endif

  va_end(ap);
  return str;
}
#undef max
#undef min
#endif

#ifdef __cplusplus
}
#endif

