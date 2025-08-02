#include <cstdint>
#include <fstream>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "codegen.h"
#include "emulator.h"
#include "wavutils.h"

LspState state;
Runtime runtime;

int main() {
  // Load program
  FILE *pgmFile = fopen("reverb_perftest.txt", "r");
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

  state.parseProgram();

  FILE *outputFile = fopen("compiled.cpp", "w");
  std::string code = convertToCode(state.instrCache);
  fprintf(outputFile, "%s", code.c_str());
  fclose(outputFile);

  // Load audio input
  std::vector<int16_t> audioSamples;
  int sampleRate = 0;
  int numChannels = 0;
  // read_wav("input.wav", audioSamples, sampleRate, numChannels);
  read_wav("input_guit.wav", audioSamples, sampleRate, numChannels);
  // read_wav("input_guit2.wav", audioSamples, sampleRate, numChannels);
  // read_wav("input_piano.wav", audioSamples, sampleRate, numChannels);
  // read_wav("sine.wav", audioSamples, sampleRate, numChannels);

  std::vector<int16_t> audioOutput;

  // Process audio samples
  for (size_t i = 0; i < audioSamples.size(); i += numChannels) {
    if (numChannels == 1) {
      runtime.audioInL = runtime.audioInR = audioSamples[i];
    } else if (numChannels == 2) {
      runtime.audioInL = audioSamples[i];
      runtime.audioInR = audioSamples[i + 1];
    }

    runtime.audioInL <<= 6;
    runtime.audioInR <<= 6;
    state.audioInL = runtime.audioInL;
    state.audioInR = runtime.audioInR;

    state.runProgram();

    runtime.runCompiled();

    runtime.audioOutL >>= 8;
    runtime.audioOutR >>= 8;
    state.audioOutL >>= 8;
    state.audioOutR >>= 8;

    if (state.audioOutL != runtime.audioOutL ||
        state.audioOutR != runtime.audioOutR) {
      printf("Audio output mismatch: %d %d vs %d %d\n", state.audioOutL,
             state.audioOutR, runtime.audioOutL, runtime.audioOutR);
      break;
    }

    audioOutput.push_back(static_cast<int16_t>(runtime.audioOutL));
    audioOutput.push_back(static_cast<int16_t>(runtime.audioOutR));
  }

  write_wav("output.wav", audioOutput, sampleRate, 2);

  return 0;
}
