#ifndef OMAP_PRINTER_C
#define OMAP_PRINTER_C
#include "../omap.h"
#include "../print.h"
REGISTER_SPECIAL_PRINTER("OMap_stringlist", stringList, {
  PUTS(L"[");
  const unsigned int length = stringList_length(&in);
  for (int i = 0; i < length; i++) {
    OMap_V el = Stringlist_getObj(&in, i);
    switch (el.t) {
    case RAW:
      USETYPEPRINTER(fptr, el.v);
      PUTS(L",");
      break;
    case OMAP:
      USENAMEDPRINTER("OMapView", el.v);
      break;
    case SLIST:
      USENAMEDPRINTER("OMap_stringlistView", el.v);
      break;
    }
  }
  PUTS(L"]");
});
REGISTER_SPECIAL_PRINTER("OMap_stringlistView", stringListView, {
  PUTS(L"[");
  const unsigned int length = stringListView_length(in);
  for (int i = 0; i < length; i++) {
    OMap_V el = StringlistView_getObj(in, i);
    switch (el.t) {
    case RAW:
      USETYPEPRINTER(fptr, el.v);
      PUTS(L",");
      break;
    case OMAP:
      USENAMEDPRINTER("OMapView", el.v);
      break;
    case SLIST:
      USENAMEDPRINTER("OMap_stringlistView", el.v);
      break;
    }
  }
  PUTS(L"]");
});
REGISTER_PRINTER(OMapView, {
  const int length = OMapView_length(in);
  PUTS(L"{");
  for (uint i = 0; i < length; i++) {
    OMap_both both = OMapView_getIndex(in, i);
    USETYPEPRINTER(fptr, both.k);
    PUTS(L":");
    switch (both.t) {
    case RAW:
      USETYPEPRINTER(fptr, both.v);
      PUTS(L";");
      break;
    case OMAP:
      USETYPEPRINTER(OMapView, both.v);
      break;
    case SLIST:
      USENAMEDPRINTER("OMap_stringlistView", both.v);
      break;
    }
  }
  PUTS(L"}");
});
REGISTER_PRINTER(OMap, {
  const int length = OMap_length(&in);
  PUTS(L"{");
  for (uint i = 0; i < length; i++) {
    OMap_both both;
    both.k = OMap_getKey(&in, i);
    OMap_V m = OMap_get(&in, both.k);
    both.t = m.t;
    both.v = m.v;
    USETYPEPRINTER(fptr, both.k);
    PUTS(L":");
    switch (both.t) {
    case RAW:
      USETYPEPRINTER(fptr, both.v);
      PUTS(L";");
      break;
    case OMAP:
      USETYPEPRINTER(OMapView, both.v);
      break;
    case SLIST:
      USENAMEDPRINTER("OMap_stringlistView", both.v);
      break;
    }
  }
  PUTS(L"}");
})
REGISTER_PRINTER(OMap_V, {
  switch (in.t) {
  case RAW: {
    PUTS(L"(RAW)");
    USETYPEPRINTER(fptr, in.v);
  } break;
  case OMAP: {
    PUTS(L"(OMAP)");
    USETYPEPRINTER(OMapView, in.v);
  } break;
  case SLIST: {
    PUTS(L"(SLIST)");
    USENAMEDPRINTER("OMap_stringlistView", in.v);
  } break;
  }
})
#endif // OMAP_PRINTER_C
