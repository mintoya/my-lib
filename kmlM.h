#ifndef KMLM_H
#define KMLM_H
#include "fptr.h"
#include "print.h"
#include "umap.h"

#define min(a, b) ((a < b) ? (a) : (b))
#define max(a, b) ((a > b) ? (a) : (b))

unsigned int kml_indexOf(fptr string, char c);
fptr kml_until(char delim, fptr string);
fptr kml_behind(char delim, fptr string);
fptr kml_inside(char limits[2], fptr string);
fptr kml_around(char limits[2], fptr string);
fptr kml_after(fptr main, um_fp slice);
fptr kml_removeSpacesPadding(fptr in);
fptr findIndex(fptr str, unsigned int index);
fptr findKey(fptr str, um_fp key);

#define fpChar(fptr) ((char *)fptr.ptr)
UMap *parse(UMap *parent, UMapList *lparent, fptr kml);

UMapList *parseList(UMapList *lparent, fptr kml) {
  if (!kml.width) {
    if (!(lparent)) {
      return UMapList_new(&defaultAllocator);
    } else {
      return lparent ? lparent : NULL;
    }
  }
  if (!(lparent)) {
    lparent = UMapList_new(&defaultAllocator);
  }
  kml = kml_removeSpacesPadding(kml);
  fptr val, pval;
  switch (fpChar(kml)[0]) {
  case '[': {
    val = kml_around("[]", kml);
    pval = kml_inside("[]", kml);
    UMapList_scoped *newmap = parseList(NULL, pval);
    if (newmap) {
      if (lparent) {
        UMapList_appendList(lparent, newmap);
      }
    }
  } break;
  case '{': {
    val = kml_around("{}", kml);
    pval = kml_inside("{}", kml);
    UMap_scoped *newmap = parse(NULL, NULL, pval);
    if (newmap) {
      if (lparent) {
        UMapList_appendChild(lparent, newmap);
      }
    }
  } break;
  case '"': {
    kml.ptr++;
    kml.width--;
    pval = (fptr){
        .width = kml_indexOf(kml, '"'),
        .ptr = kml.ptr,
    };
    val = pval;
    if (pval.width == kml.width) {
      pval.width--;
      val.width--;
    } else {
      val.width++;
    }
    kml.ptr--;
    kml.width++;
    UMapList_append(lparent, pval);
  } break;
  default: {
    val = kml_behind(',', kml);
    pval = kml_until(',', kml);
    UMapList_append(lparent, pval);
  }
  }
  fptr next = kml_after(kml, val);
  next = kml_removeSpacesPadding(next);
  if (next.width && fpChar(next)[0] == ',') {
    next.width--;
    next.ptr++;
  }
  next = kml_removeSpacesPadding(next);
  return parseList(lparent, next);
}
UMap *parse(UMap *parent, UMapList *lparent, fptr kml) {
  kml = kml_removeSpacesPadding(kml);
  if (!kml.width) {
    if (!(parent || lparent)) {
      return UMap_new(&defaultAllocator);
    } else {
      return parent ? parent : NULL;
    }
  }
  if (!(parent || lparent)) {
    parent = UMap_new(&defaultAllocator);
  }
  if (fpChar(kml)[0] == '{') {
    kml = kml_inside("{}", kml);
  }
  kml = kml_removeSpacesPadding(kml);
  if (!kml.width) {
    if (!(parent || lparent)) {
      return UMap_new(&defaultAllocator);
    } else {
      return parent ? parent : NULL;
    }
  }

  if (kml_indexOf(kml, ':') == kml.width) {
    return parent ? parent : NULL;
  }
  fptr key = kml_until(':', kml);
  fptr val = kml_after(kml, kml_behind(':', kml)), pval;

  val = kml_removeSpacesPadding(val);
  key = kml_removeSpacesPadding(key);

  switch (fpChar(val)[0]) {
  case '[': {
    val = kml_around("[]", val);
    pval = kml_inside("[]", val);
    UMapList_scoped *newmap = parseList(NULL, pval);
    if (newmap) {
      if (lparent) {
        UMapList_appendList(lparent, newmap);
      }
      if (parent) {
        UMap_setList(parent, key, newmap);
      }
    }
  } break;
  case '{': {
    val = kml_around("{}", val);
    pval = kml_inside("{}", val);
    UMap_scoped *newmap = parse(NULL, NULL, pval);
    if (newmap) {
      if (lparent) {
        UMapList_appendChild(lparent, newmap);
      }
      if (parent) {
        UMap_setChild(parent, key, newmap);
      }
    }
  } break;
  case '"': {
    val.ptr++;
    val.width--;
    pval = kml_until('"', val);
    val = pval;
    val.width++;
    UMap_set(parent, key, pval);
  } break;
  default: {
    val = kml_behind(';', val);
    pval = kml_until(';', val);
    UMap_set(parent, key, pval);
  }
  }
  return parse(parent, lparent, kml_after(kml, val));
}

#endif // KMLM_H
#ifdef KMLM_C

// index after end if c not present
unsigned int kml_indexOf(um_fp string, char c) {
  int i;
  char *ptr = (char *)string.ptr;
  for (i = 0; i < string.width && ptr[i] != c; i++)
    ;
  return i;
}
um_fp kml_inside(char limits[2], um_fp string) {
  if (!string.width)
    return nullUmf;
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
          return (um_fp){
              .width = (size_t)ii - 1,
              .ptr = ((uint8_t *)string.ptr) + i + 1,
          };
        // if (i + ii == string.width - 1)
        //   return (um_fp){.ptr = ((uint8_t *)string.ptr) + i + 1,
        //                  .width = (size_t)ii - 1};
        if (i + ii == string.width - 1)
          return (um_fp){
              .width = (size_t)ii,
              .ptr = ((uint8_t *)string.ptr) + i + 1,
          };
      }
    }
  }
  return nullUmf;
}

um_fp kml_around(char limits[2], um_fp string) {
  if (!string.width)
    return nullUmf;
  char front = limits[0];
  char back = limits[1];

  int in_single = 0;
  int in_double = 0;

  for (int i = 0; i < string.width; i++) {
    char c = ((char *)string.ptr)[i];
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
          return (um_fp){
              .width = (size_t)ii + 1,
              .ptr = ((uint8_t *)string.ptr) + i,
          };
        if (i + ii == string.width - 1)
          return (um_fp){
              .width = (size_t)ii + 1,
              .ptr = ((uint8_t *)string.ptr) + i,
          };
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
    return nullUmf;
  char *mainStart = (char *)main.ptr;
  char *mainEnd = mainStart + main.width;
  char *sliceStart = (char *)slice.ptr;
  char *sliceEnd = sliceStart + slice.width;

  // assert(sliceStart >= mainStart && sliceEnd <= mainEnd);
  if (!(sliceStart >= mainStart && sliceEnd <= mainEnd))
    return nullUmf;

  return (um_fp){
      .width = (size_t)(mainEnd - sliceEnd),
      .ptr = (uint8_t *)sliceEnd,
  };
}
#define stack_split(result, string, ...)                                   \
  result = (um_fp *)alloca(                                                \
      sizeof(um_fp) *                                                      \
      (sizeof((unsigned int[]){__VA_ARGS__}) / sizeof(unsigned int) + 1)   \
  );                                                                       \
  do {                                                                     \
    uint8_t *last;                                                         \
    unsigned int args[] = {__VA_ARGS__};                                   \
    for (int i = 0; i < sizeof(args) / sizeof(unsigned int); i++) {        \
      args[i] = (i == 0) ? (min(args[i], string.width))                    \
                         : (min(string.width, max(args[i], args[i - 1]))); \
      result[i] = (um_fp){                                                 \
          .width = (i == 0) ? (args[0]) : (args[i] - args[i - 1]),         \
          .ptr = (i == 0) ? (string.ptr) : (last),                         \
      };                                                                   \
      last = ((uint8_t *)result[i].ptr) + result[i].width;                 \
    }                                                                      \
    result[sizeof(args) / sizeof(unsigned int)] = (um_fp){                 \
        .width = string.width - ((uint8_t *)last - (uint8_t *)string.ptr), \
        .ptr = last,                                                       \
    };                                                                     \
  } while (0);
char isSkip(char c) {
  switch (c) {
  case ' ':
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
#undef min
#undef max
#endif
