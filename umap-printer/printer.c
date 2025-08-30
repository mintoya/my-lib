#include "printer.h"
#include <stdio.h>
#include <string.h>

#define and &&
#define or ||
#define min(a, b) ((a < b) ? (a) : (b))

void print_int(const void *integer) { printf("%i", *(int *)integer); }
void print_chars(const void *charf) { printf("%s", *(char **)charf); }
void print_umfp(const void *umfp) {
  if ((*(um_fp *)umfp).length && (*(um_fp *)umfp).ptr)
    printf("%.*s", (*(um_fp *)umfp).length, (char *)(*(um_fp *)umfp).ptr);
}
void print_rawList(const void *ref) {
  List *l = *(List **)ref;
  printf("{");
  for (int i = 0; i < l->length; i++) {
    const char *refw = List_gst(l, i);
    printf("0x");
    for (int ii = l->width / sizeof(char) - 1; ii >= 0; ii--) {
      printf("%02x", *((unsigned char *)refw + ii));
    }
    printf(", ");
  }
  printf("}");
}
void print_UMap(const void *ref) {
  UMap *u = *(UMap **)ref;
  printf("{\n");
  for (int i = 0; i < u->keyReg->length; i++) {
    um_fp key = UMap_getKeyAtIndex(u, i);
    um_fp val = UMap_getValAtKey(u, key);
    if (val.length && val.ptr) {
      printf("\t %.*s :", key.length, (char *)key.ptr);
      printf("0x");
      for (int ii = val.length / sizeof(char) - 1; ii >= 0; ii--) {
        printf("%02x", *((unsigned char *)val.ptr + ii));
      }
      printf("\n");
    }
  }
  printf("}\n");
}
typedef void (*printer)(const void *);

char *defined[] = {"int", "char *", "um_fp", "List *", "UMap *"};

List *typeList = NULL; // pointers to places in definedRef
List *typeBuf = NULL;

printer definedCallers[] = {print_int, print_chars, print_umfp, print_rawList,
                            print_UMap};
List *callers = NULL; // function pointers

void printerInit() {
  if (!typeList) {
    typeList = mList(void *);
    typeBuf = mList(char);
    callers = mList(printer *);
  }
}
void typeAdd(char *typeName) {
  List_append(typeList,
              (void *[1]){typeBuf->head + typeBuf->length * typeBuf->width});
  List_appendFromArr(typeBuf, typeName, strlen(typeName));
  List_append(typeBuf, (char[1]){0});
}
void fnAdd(printer *fn) { List_append(callers, fn); }
void registerPrinter(char *type, printer *fn) {
  printerInit();
  typeAdd(type);
  fnAdd(fn);
}

void mPrint(char *typeName, const void *ref) {
  printerInit();
  for (int i = 0; i < sizeof(defined) / sizeof(char *); i++) {
    if (!strncmp(defined[i], typeName,
                 min(strlen(typeName), strlen(defined[i])))) {
      definedCallers[i](ref);
    }
  }
  if (typeList) {
    mList_forEach(typeList, char *, tn, {
      if (!strncmp(tn, typeName, min(strlen(typeName), strlen(tn)))) {
        printer fn = mList_get(callers, printer, _i);
        fn(ref);
      }
    });
  }
}

#undef min
#undef and
