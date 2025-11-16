#ifndef KML_PARSER_H
#define KML_PARSER_H
#include "um_fp.h"
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
um_fp kml_behind(char delim, um_fp string);
um_fp kml_inside(char limits[2], um_fp string);
um_fp kml_around(char limits[2], um_fp string);
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
    next.ptr = (uint8_t *)next.ptr + 1;
    next.width--;

    value = kml_removeSpacesPadding(kml_until('"', next));
    toParse = kml_behind('"', next);

    toParse.ptr = (uint8_t *)toParse.ptr - 1;
    toParse.width++;

    next.ptr = (uint8_t *)next.ptr - 1;
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

#endif
