#include <stdint.h>
#define cSum_REDUNDANCY_AMMOUNT 4
#include "cSum.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MY_LIST_C
#include "../my-list/my-list.h"
#define total_messages ((uint32_t)8640000)

// Flip a single random bit in the buffer
void flip_random_bit(void *buf, size_t len) {
  if (len == 0)
    return;
  int iter = rand() % len + 1;
  for (int i = 0; i < iter; i++) {
    size_t byte_index = rand() % len;
    uint8_t bit_index = rand() % 8;
    ((uint8_t *)buf)[byte_index] ^= 1 << bit_index;
  }
}

int main(void) {
  srand((unsigned)time(NULL));
  dataChecker d = cSum_new();

  um_fp original = um_from("abcdefghijklmn");

  clock_t start = clock();

  // Benchmark checksum creation
  for (uint32_t i = 0; i < total_messages; i++) {
    checkData c = cSum_toSum(d, original);
    assert(um_eq(cSum_fromSum(c), original));
  }

  clock_t mid = clock();

  int caught_errors = 0, fake_errors = 0;
  for (uint32_t i = 0; i < total_messages; i++) {
    checkData c = cSum_toSum(d, original);
    flip_random_bit(c.data.ptr, c.data.width);
    um_fp result = cSum_fromSum(c);

    if (!result.ptr) {
      caught_errors++;
    } else if (um_eq(result, original)) {
      caught_errors++;
      fake_errors++;
    }
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
  printf("False errors: %d\n", fake_errors);
  printf("Total uncaught: %d\n", total_messages - caught_errors);
  printf("data Speed: %f Kb/ms",
         total_messages * original.width / verify_sec / 1000 / 1000);
  cSum_free(d);
  return 0;
}
