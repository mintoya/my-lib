#define PRINTER_LIST_TYPENAMES

#include "hmap.h"
#include "print.h"
#include "um_fp.h"
#include "wheels.h"

#include <stdint.h>  // For uintptr_t
#include <stdio.h>   // For printf
#include <stdlib.h>  // For malloc/free, exit
#include <string.h>  // For sprintf
#include <windows.h> // For Windows high-resolution timer

// A helper function to get high-resolution time in nanoseconds (WINDOWS
// VERSION)
long long get_nanoseconds() {
  // This static variable will hold the timer frequency.
  // It's initialized only once on the first call.
  static LARGE_INTEGER frequency;
  if (frequency.QuadPart == 0) {
    if (!QueryPerformanceFrequency(&frequency)) {
      fprintf(stderr, "Error: QueryPerformanceFrequency failed!\n");
      fprintf(stderr, "This system does not support high-resolution timing.\n");
      exit(EXIT_FAILURE);
    }
  }

  LARGE_INTEGER ticks;
  QueryPerformanceCounter(&ticks);

  // Convert ticks to nanoseconds.
  // (ticks.QuadPart * 1e9) / frequency.QuadPart
  // We multiply by 1_000_000_000.0 (as a double) first to maintain precision
  // before dividing by the frequency.
  return (long long)((double)ticks.QuadPart * 1000000000.0 /
                     (double)frequency.QuadPart);
}

int main() {
  // 1. SETUP: Create 1 million items
  const size_t N = (size_t)(HMap_MAXHASH);
  printf("Setting up %zu key-value pairs...\n", N);

  // Pre-calculate all um_fp keys and values as requested
  um_fp *keys = malloc(N * sizeof(um_fp));
  um_fp *values = malloc(N * sizeof(um_fp));
  char **key_strings = malloc(N * sizeof(char *));   // To hold string data
  char **value_strings = malloc(N * sizeof(char *)); // To hold string data

  if (!keys || !values || !key_strings || !value_strings) {
    fprintf(stderr, "Failed to allocate memory for setup.\n");
    return 1;
  }

  for (size_t i = 0; i < N; i++) {
    key_strings[i] = malloc(50);
    value_strings[i] = malloc(50);
    if (!key_strings[i] || !value_strings[i]) {
      fprintf(stderr, "Failed to allocate string memory.\n");
      return 1;
    }

    sprintf(key_strings[i], "key%zu", i);
    keys[i] =
        um_from(key_strings[i]); // Call um_from *outside* the benchmark loop

    sprintf(value_strings[i], "value%zu", i);
    values[i] = um_from(value_strings[i]);
  }
  printf("Setup complete.\n");

  printf("Populating maps...\n");
  HMap_scoped *hm = HMap_new();
  UMap_scoped *um = UMap_new();

  for (size_t i = 0; i < N; i++) {
    HMap_set(hm, keys[i], values[i]);
    printf("populated hm %i\n", i);
    UMap_set(um, keys[i], values[i]);
    printf("populated um %i\n", i);
  }
  printf("done populating");

  printf("Maps populated.\n");
  printf("HMap footprint: %zu\n", HMap_footprint(hm));
  printf("UMap footprint: %zu\n\n", UMap_footprint(um));

  // 3. BENCHMARK HMAP
  printf("--- Benchmarking HMap_get() ---\n");
  long long start_hm = get_nanoseconds();

  // Use volatile to prevent compiler from optimizing the loop away
  volatile uintptr_t sum_hm = 0;
  for (size_t i = 0; i < N; i++) {
    um_fp result = HMap_get(hm, keys[i]);
    sum_hm += (uintptr_t)result.ptr; // "Use" the result
  }

  long long end_hm = get_nanoseconds();
  double time_hm = (end_hm - start_hm) / 1e9; // duration in seconds

  printf("HMap total time for %zu gets: %.6f s\n", N, time_hm);
  printf("HMap average time per get: %.3f ns\n\n", (time_hm * 1e9) / N);

  // 4. BENCHMARK UMAP
  printf("--- Benchmarking UMap_get() ---\n");
  long long start_um = get_nanoseconds();

  volatile uintptr_t sum_um = 0;
  for (size_t i = 0; i < N; i++) {
    um_fp result = UMap_get(um, keys[i]);
    sum_um += (uintptr_t)result.ptr; // "Use" the result
  }

  long long end_um = get_nanoseconds();
  double time_um = (end_um - start_um) / 1e9; // duration in seconds

  printf("UMap total time for %zu gets: %.6f s\n", N, time_um);
  printf("UMap average time per get: %.3f ns\n\n", (time_um * 1e9) / N);

  // Print checksums to ensure loops ran
  printf("HMap checksum (volatile): %lu\n", (unsigned long)sum_hm);
  printf("UMap checksum (volatile): %lu\n", (unsigned long)sum_um);

  // 5. CLEANUP
  printf("Cleaning up...\n");
  for (size_t i = 0; i < N; i++) {
    free(key_strings[i]);
    free(value_strings[i]);
  }
  free(keys);
  free(values);
  free(key_strings);
  free(value_strings);

  // _scoped maps will free themselves here
  return 0;
}
