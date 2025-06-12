#include <chrono>
#include <cstdint>
#include <fstream>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "emulator.h"

LspState state;

static constexpr long long testLength = 2000000;
static constexpr long long nTests = 5;

int main() {
  FILE *pgmFile = fopen("/Users/giuliozausa/personal/programming/lsp/emulator/reverb_perftest.txt", "r");
  if (!pgmFile) {
    fprintf(stderr, "Error opening program file.\n");
    return 1;
  }
  char line[512] = {0};
  int iramIdx = 0x80;
  while (fgets(line, sizeof(line), pgmFile)) {
    int b1, b2, b3;
    const char *format = "%2x %2x %2x";
    if (line[4] == ':') {
      format = "%*[^:]: %2x %2x %2x";
    }
    if (sscanf(line, format, &b1, &b2, &b3) == 3) {
      int32_t value = (b1 << 16) | (b2 << 8) | b3;
      state.iram[iramIdx++] = value;
    }
  }
  fclose(pgmFile);

  int sumTimes = 0;

  for (size_t j = 0; j < nTests; j++) {
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < testLength; i += 1) {
      state.audioInL = rand();
      state.audioInR = rand();

      state.runProgram();
    }

    auto stop = std::chrono::high_resolution_clock::now();

    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    printf("%lld iteration: %lld ms\n", testLength, duration.count());

    sumTimes += duration.count();
  }

  printf("Average time for %lld iterations: %lld ms   %.2fx rt\n", testLength,
         sumTimes / nTests, (double)testLength / 44.1 / (sumTimes / nTests));

  return 0;
}
