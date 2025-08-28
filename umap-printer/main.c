#include "printer.h"
#include <stdio.h>

typedef struct{
  int a;
  char b;
}ic;

void print_ic(const void* ref){
  ic t = *(ic*)ref;
  printf("ic:{ a:%i b:%c}",t.a,t.b);
}
void printListofInts(const void* ref){
  List* l = *(List**)ref;
  printf("{");
  $List_forEach(l,int,i,
    {
      printf("%i ,",i);
    }
  );
  printf("}\n");
}

int main(void) {
  addPrinter(ic,print_ic);
  registerPrinter("List<int>",(printer[1]){printListofInts} );
  
  ic a = {.a = 2,.b = '2'};
  ic b = {.a = 55,.b = 'c'};


  usePrint(ic,a); printf("\n");
  usePrint(ic,b); printf("\n");
  List* l = $List(int);
  List_append(l,(int[1]){1});
  List_append(l,(int[1]){2});
  List_append(l,(int[1]){18});
  List_append(l,(int[1]){0xFF<<(4)});
  List_append(l,(int[1]){17});
  mPrint("List<int>",&l);
  usePrint(List*, l);

  List* l2 = $List(char);
  List_appendFromArr(l2,"hello world",12);
  printf("%s\n",l2->head);
  usePrint(List*, l2);

  return 0;
}
