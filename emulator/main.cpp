#include <cstdint>
#include <fstream>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "emulator.h"
#include "wavutils.h"

LspState state;

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

  state.optimiseProgram();
  
  // Process audio samples
  for (size_t i = 0; i < audioSamples.size(); i += numChannels) {
    if (numChannels == 1) {
      state.audioInL = state.audioInR = audioSamples[i];
    } else if (numChannels == 2) {
      state.audioInL = audioSamples[i];
      state.audioInR = audioSamples[i + 1];
    }

    state.audioInL <<= 6;
    state.audioInR <<= 6;

    state.runProgram();

    state.audioOutL >>= 8;
    state.audioOutR >>= 8;

    audioOutput.push_back(static_cast<int16_t>(state.audioOutL));
    audioOutput.push_back(static_cast<int16_t>(state.audioOutR));
  }

  write_wav("output.wav", audioOutput, sampleRate, 2);

  return 0;
}
