#include "pico/stdlib.h"
#include <cstdlib>
#include <stdio.h>

#include "runtime.h"

static constexpr long long testLength = 500000;

Runtime runtime;

int main() {
  set_sys_clock_khz(270000, true);

  stdio_init_all();

  while (true) {
    runtime.audioInL = rand();
    runtime.audioInR = rand();

    uint64_t start = time_us_64();

    for (size_t i = 0; i < testLength; i += 1) {
      runtime.runCompiled();
    }

    uint64_t stop = time_us_64();

    uint64_t duration = (double)(stop - start) / 1000.0;

    printf("%lld iteration: %lld ms\n", testLength, duration);
    printf("Average time for %lld iterations: %lld ms   %.2fx rt\n", testLength,
           duration, (double)testLength / 44.1 / duration);
  }

  return 0;
}
