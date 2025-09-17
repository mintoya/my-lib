#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "cSum.h"
#define MY_LIST_C
#include "../my-list/my-list.h"

// Flip a single random bit in the buffer
void flip_random_bit(void *buf, size_t len) {
    if (len == 0) return;
    size_t byte_index = rand() % len;
    uint8_t bit_index = rand() % 8;
    ((uint8_t*)buf)[byte_index] ^= 1 << bit_index;
}

int main(void) {
    srand((unsigned)time(NULL));

    const char *text = "The quick brown fox jumps over the lazy dog";
    um_fp original = um_from(text);

    dataChecker d = cSum_new();

    const int total_messages = 1000000; // 1 million messages
    clock_t start = clock();

    // Benchmark checksum creation
    for (int i = 0; i < total_messages; i++) {
        checkData c = cSum_toSum(d, original);
        (void)c; // avoid unused variable warning
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
    printf("Messages created: %d in %.3f s (%.0f msg/s)\n",
           total_messages, create_sec, total_messages / create_sec);
    printf("Messages verified with random bit flips: %d in %.3f s (%.0f msg/s)\n",
           total_messages, verify_sec, total_messages / verify_sec);
    printf("Errors caught: %d / %d (%.2f%%)\n",
           caught_errors, total_messages, caught_errors * 100.0 / total_messages);

    cSum_free(d);
    return 0;
}

