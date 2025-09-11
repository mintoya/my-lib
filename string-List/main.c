#include "../umap-printer/printer.h"
#include "um_fp.h"
#define STRING_LIST_C
#include "stringList.h"
#define MY_LIST_C
#include "../my-list/my-list.h"
void stringList_printMeta(const stringList *sl) {
    if (!sl) {
        printf("stringList (null)\n");
        return;
    }

    List *metaList = (List*)( (void*)( &sl->List_stringMetaData ) );
    printf("stringList metadata (length=%u):\n", metaList->length);

    for (unsigned int i = 0; i < metaList->length; i++) {
        stringMetaData *meta = (stringMetaData *)List_getRef(metaList, i);
        if (!meta) {
            printf("  [%u] <null>\n", i);
            continue;
        }
        printf("  [%u] index=%u, width=%u, size=%u\n",
               i, meta->index, meta->width, meta->_size);
    }
}
int main(void)
{
  stringList* l = stringList_new();
  stringList_append(l, um_from("hello world "));
  stringList_append(l, um_from("very long string test"));
  stringList_append(l, um_from("very long string test"));
  stringList_append(l, um_from("very long string test"));
  stringList_insert(l, um_from("hello world 1"),0);
  stringList_set(l, um_from("hhello worldhello worldhello worldhello worldello world 0"),0);
  stringList_set(l,um_from("hello world 2"),2);
  stringList_set(l,um_fromT(int,0x63),2);
  stringList_set(l, um_from("abc"),0);


  stringList_printMeta(l);
  return EXIT_SUCCESS;
}
