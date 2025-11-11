#ifndef KMLM_H
#define KMLM_H
#include "kml.h"
#include "print.h"
#include "um_fp.h"
#include "umap.h"

#define fpChar(um_fp) ((char *)um_fp.ptr)
UMap *parse(UMap *parent, UMapList *lparent, um_fp kml);

UMapList *parseList(UMapList *lparent, um_fp kml) {
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
  um_fp val, pval;
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
    pval = (um_fp){
        .ptr = kml.ptr,
        .width = kml_indexOf(kml, '"'),
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
  um_fp next = kml_after(kml, val);
  next = kml_removeSpacesPadding(next);
  if (next.width && fpChar(next)[0] == ',') {
    next.width--;
    next.ptr++;
  }
  next = kml_removeSpacesPadding(next);
  return parseList(lparent, next);
}
UMap *parse(UMap *parent, UMapList *lparent, um_fp kml) {
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
  um_fp key = kml_until(':', kml);
  um_fp val = kml_after(kml, kml_behind(':', kml)), pval;

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
