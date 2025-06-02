#include <cstdint>
#include <fstream>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "emulator.h"

// Reads a 16-bit PCM mono or stereo WAV file into a vector of int16_t.
// Returns true on success, false on failure.
bool read_wav(const std::string &filename, std::vector<int16_t> &samples,
              int &sampleRate, int &numChannels) {
  samples.clear();
  std::ifstream file(filename, std::ios::binary);
  if (!file)
    return false;

  char riff[4];
  file.read(riff, 4);
  if (std::string(riff, 4) != "RIFF")
    return false;
  file.ignore(4); // file size
  char wave[4];
  file.read(wave, 4);
  if (std::string(wave, 4) != "WAVE")
    return false;

  // Read chunks
  bool fmtFound = false, dataFound = false;
  int bitsPerSample = 0;
  uint32_t dataSize = 0;
  while (file && (!fmtFound || !dataFound)) {
    char chunkId[4];
    file.read(chunkId, 4);
    uint32_t chunkSize = 0;
    file.read(reinterpret_cast<char *>(&chunkSize), 4);

    if (std::string(chunkId, 4) == "fmt ") {
      uint16_t audioFormat = 0;
      file.read(reinterpret_cast<char *>(&audioFormat), 2);
      file.read(reinterpret_cast<char *>(&numChannels), 2);
      file.read(reinterpret_cast<char *>(&sampleRate), 4);
      file.ignore(6); // byte rate + block align
      file.read(reinterpret_cast<char *>(&bitsPerSample), 2);
      file.ignore(chunkSize - 16);
      if (audioFormat != 1 || bitsPerSample != 16)
        return false;
      fmtFound = true;
    } else if (std::string(chunkId, 4) == "data") {
      dataSize = chunkSize;
      samples.resize(dataSize / 2);
      file.read(reinterpret_cast<char *>(samples.data()), dataSize);
      dataFound = true;
    } else {
      file.ignore(chunkSize);
    }
  }
  return fmtFound && dataFound;
}

// Writes a 16-bit PCM mono or stereo WAV file from a vector of int16_t.
// Returns true on success, false on failure.
bool write_wav(const std::string &filename, const std::vector<int16_t> &samples,
               int sampleRate, int numChannels) {
  std::ofstream file(filename, std::ios::binary);
  if (!file)
    return false;

  uint32_t dataChunkSize = samples.size() * sizeof(int16_t);
  uint32_t fmtChunkSize = 16;
  uint32_t riffChunkSize = 4 + (8 + fmtChunkSize) + (8 + dataChunkSize);

  // RIFF header
  file.write("RIFF", 4);
  file.write(reinterpret_cast<const char *>(&riffChunkSize), 4);
  file.write("WAVE", 4);

  // fmt chunk
  file.write("fmt ", 4);
  file.write(reinterpret_cast<const char *>(&fmtChunkSize), 4);
  uint16_t audioFormat = 1;
  file.write(reinterpret_cast<const char *>(&audioFormat), 2);
  file.write(reinterpret_cast<const char *>(&numChannels), 2);
  file.write(reinterpret_cast<const char *>(&sampleRate), 4);
  uint32_t byteRate = sampleRate * numChannels * 2;
  uint16_t blockAlign = numChannels * 2;
  uint16_t bitsPerSample = 16;
  file.write(reinterpret_cast<const char *>(&byteRate), 4);
  file.write(reinterpret_cast<const char *>(&blockAlign), 2);
  file.write(reinterpret_cast<const char *>(&bitsPerSample), 2);

  // data chunk
  file.write("data", 4);
  file.write(reinterpret_cast<const char *>(&dataChunkSize), 4);
  file.write(reinterpret_cast<const char *>(samples.data()), dataChunkSize);

  return true;
}

LspState state;

int main() {
  // Load program
  FILE *pgmFile = fopen("spectrum_test.txt", "r");
  // FILE *pgmFile = fopen("../lsp_pgm/stereo_eq.txt", "r");
  // FILE *pgmFile = fopen("../lsp_pgm/enhancer.txt", "r");
  // FILE *pgmFile = fopen("../lsp_pgm/thru.txt", "r");
  // FILE *pgmFile = fopen("lsp_square.txt", "r");
  // FILE *pgmFile = fopen("../test.txt", "r");
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
      // if (sscanf(line, "%2x %2x %2x", &b1, &b2, &b3) == 3) {
      int32_t value = (b1 << 16) | (b2 << 8) | b3;
      state.iram[iramIdx++] = value;
    }
  }
  fclose(pgmFile);

  // Load audio input
  std::vector<int16_t> audioSamples;
  int sampleRate = 0;
  int numChannels = 0;
  read_wav("input_piano.wav", audioSamples, sampleRate, numChannels);

  std::vector<int16_t> audioOutput;

  // Process audio samples
  for (size_t i = 0; i < audioSamples.size(); i += numChannels) {
    if (numChannels == 1) {
      state.audioInL = state.audioInR = audioSamples[i];
    } else if (numChannels == 2) {
      state.audioInL = audioSamples[i];
      state.audioInR = audioSamples[i + 1];
    }

    // state.audioInL <<= 6;
    // state.audioInR <<= 6;

    state.runProgram();

    // state.audioOutL >>= 8;
    // state.audioOutR >>= 8;

    audioOutput.push_back(static_cast<int16_t>(state.audioOutL));
    audioOutput.push_back(static_cast<int16_t>(state.audioOutR));
  }

  write_wav("output.wav", audioOutput, sampleRate, 2);

  // state.audioInL = state.audioInR = sign_extend_24(0xffffc0);
  // for (size_t i = 0; i < 1; i++)
  //   state.runProgram();

  // for (size_t i = 0; i < 0x80; i++) {
  //   printf("%04x: %06x\n", i, state.iram[i] & 0xffffff);
  // }

  return 0;
}
