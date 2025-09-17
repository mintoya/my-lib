#include "cSum.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MY_LIST_C
#include "../my-list/my-list.h"

// Flip a single random bit in the buffer
void flip_random_bit(void *buf, size_t len) {
  static List *l = NULL;
  if (!l)
    l = mList(unsigned int);
  int iter = rand() % 15 + 1;
  l->length = 0;
  for (int i = 0; i < iter; i++) {
    if (len == 0)
      return;
    size_t byte_index = rand() % len;
    if (List_search(l, &byte_index) < 0) {
      uint8_t bit_index = rand() % 8;
      ((uint8_t *)buf)[byte_index] ^= 1 << bit_index;
    }
  }
}

int main(void) {
  srand((unsigned)time(NULL));
  dataChecker d = cSum_new();

  um_fp original = um_from(
      "The quick brown fox jumps over the lazy dog"
      "The quick brown fox jumps over the lazy dog"
      "The quick brown fox jumps over the lazy dog"
      );

  const int total_messages = 10000000; // 1 million messages
  clock_t start = clock();

  // Benchmark checksum creation
  for (int i = 0; i < total_messages; i++) {
    checkData c = cSum_toSum(d, original);
    assert(um_eq(cSum_fromSum(c), original));
  }

  clock_t mid = clock();

  // Benchmark checksum verification with random bit flips
  int caught_errors = 0;
  for (int i = 0; i < total_messages; i++) {
    checkData c = cSum_toSum(d, original);
    flip_random_bit(c.data.ptr, c.data.width);
    um_fp result = cSum_fromSum(c);
    if (result.ptr == 0 && result.width == 0)
      caught_errors++;
  }

  clock_t end = clock();

  double create_sec = (double)(mid - start) / CLOCKS_PER_SEC;
  double verify_sec = (double)(end - mid) / CLOCKS_PER_SEC;

  printf("Benchmark results:\n");
  printf("Messages created: %d in %.3f s (%.0f msg/s)\n", total_messages,
         create_sec, total_messages / create_sec);
  printf("Messages verified with random bit flips: %d in %.3f s (%.0f msg/s)\n",
         total_messages, verify_sec, total_messages / verify_sec);
  printf("Errors caught: %d / %d (%.2f%%)\n", caught_errors, total_messages,
         caught_errors * 100.0 / total_messages);

  cSum_free(d);
  return 0;
}
