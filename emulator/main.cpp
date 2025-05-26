#include <cstdint>
#include <fstream>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

// Performs signed 24-bit addition with saturation.
// Inputs are assumed to be signed 24-bit values stored in int32_t.
// Returns the saturated signed 24-bit result as int32_t.
int32_t add24_sat(int32_t a, int32_t b) {
  // Mask to 24 bits and sign-extend
  auto sign_extend_24 = [](int32_t x) -> int32_t {
    x &= 0xffffff;
    if (x & 0x800000) // If sign bit is set
      x |= ~0xffffff;
    return x;
  };

  a = sign_extend_24(a);
  b = sign_extend_24(b);

  int64_t sum = static_cast<int64_t>(a) + static_cast<int64_t>(b);

  // 24-bit signed min/max
  constexpr int32_t MIN24 = -0x800000;
  constexpr int32_t MAX24 = 0x7fffff;

  if (sum > MAX24)
    return MAX24;
  if (sum < MIN24)
    return MIN24;
  return static_cast<int32_t>(sum);
}

class LspState {
public:
  // Public audio inteface
  int32_t audioInL = 0;
  int32_t audioInR = 0;
  int32_t audioOutL = 0;
  int32_t audioOutR = 0;

  // Internal state
  int32_t accA = 0;
  int32_t accB = 0;
  uint8_t bufferPos = 0;
  int32_t iram[0x200] = {0};
  int32_t audioIn = 0;
  int32_t audioOut = 0;

  // For pipelining
  int32_t accAHistory[8] = {0};
  int32_t accBHistory[8] = {0};
  uint8_t pipelinePos = 0;

  void runProgram() {
    for (size_t pc = 0x80; pc < 0x200; pc++) {
      if (pc >= (0x80 + 384 / 2))
        audioIn = audioInL;
      else
        audioIn = audioInR;

      uint32_t instr = iram[pc];
      pipelinePos = (pipelinePos + 1) & 0x7;
      step(instr);
      accAHistory[pipelinePos] = accA;
      accBHistory[pipelinePos] = accB;

      if (pc >= (0x80 + 384 / 2))
        audioOutL = audioOut;
      else
        audioOutR = audioOut;
    }

    bufferPos = (bufferPos - 1) & 0x7f;
  }

  int32_t getAccAForStore() { return accAHistory[(pipelinePos - 3) & 0x7]; }
  int32_t getAccBForStore() { return accBHistory[(pipelinePos - 3) & 0x7]; }

  void step(uint32_t instr) {
    if (instr == 0) {
      // No operation, just return
      return;
    }

    // Decode
    uint8_t ii = (instr >> 16) & 0xff;
    uint8_t rr = (instr >> 8) & 0xff;
    int8_t cc = (int8_t)(instr & 0xff);

    uint8_t opcode = ii & 0xe0;
    uint8_t writeCtrl = ii & 0x18;
    uint8_t extRamCtrl = ii & 0x7;

    uint8_t shifter = (rr & 0x80) != 0;
    uint8_t memOffset = rr & 0x7f;

    // Dest/src ram position
    uint32_t ramPos = ((uint32_t)memOffset + bufferPos) & 0x7f;

    // Debug
    // printf("op:%x wr:%x er:%x sh:%x mo:%02x cf:%i\n", opcode, writeCtrl,
    // extRamCtrl, shifter, memOffset, cc);

    // Write ram
    if (writeCtrl == 0x08) {
      iram[ramPos] = getAccAForStore();
    } else if (writeCtrl == 0x10) {
      iram[ramPos] = getAccBForStore();
    } else if (writeCtrl == 0x18) {
      iram[ramPos] = getAccAForStore(); // ??
    }

    // Audio output
    if (ii == 0xc8 && memOffset == 0x58) {
      audioOut = getAccAForStore();
    }

    // Multiply
    int32_t multA = iram[ramPos];
    if (opcode == 0xc0) {
      if (memOffset == 0x50)
        multA = accA;
      else if (memOffset == 0x7e || memOffset == 0x7f)
        multA = audioIn;
    }
    int32_t multB = cc;
    int32_t multRes = multA * multB;
    multRes >>= shifter ? 5 : 7;

    // Constant load
    bool useConstant =
        memOffset == 1 | memOffset == 2 || memOffset == 3 || memOffset == 4;
    int32_t ccLoad = cc;
    if (memOffset == 2)
      ccLoad <<= 5;
    else if (memOffset == 3)
      ccLoad <<= 10;
    else if (memOffset == 4)
      ccLoad <<= 15;
    ccLoad <<= shifter ? 2 : 0;

    if (useConstant) {
      multRes = ccLoad;
    }

    // Accumulate
    if (opcode == 0x00) {
      accA = add24_sat(accA, multRes);
    } else if (opcode == 0x20) {
      accA = add24_sat(0, multRes);
    } else if (opcode == 0x40) {
      accB = add24_sat(accB, multRes);
    } else if (opcode == 0x60) {
      accB = add24_sat(0, multRes);
    } else if (opcode == 0x80) {
      // ??
      printf("Unimplemented opcode: %02x\n", opcode);
    } else if (opcode == 0xa0) {
      // ??
      accA = add24_sat(0, multRes);
    } else if (opcode == 0xc0) {
      // ??
      if (memOffset != 0x58)
        accA = add24_sat(0, multRes);
    } else if (opcode == 0xe0) {
      // ??
      printf("Unimplemented opcode: %02x\n", opcode);
    } else {
      printf("Unknown opcode: %02x\n", opcode);
    }
  }
};

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
  // FILE *pgmFile = fopen("../lsp_pgm/stereo_eq.txt", "r");
  FILE *pgmFile = fopen("spectrum_test.txt", "r");
  // FILE *pgmFile = fopen("../lsp_pgm/thru.txt", "r");
  // FILE *pgmFile = fopen("../test.txt", "r");
  if (!pgmFile) {
    fprintf(stderr, "Error opening program file.\n");
    return 1;
  }
  char line[512] = {0};
  int iramIdx = 0x80;
  while (fgets(line, sizeof(line), pgmFile)) {
    int b1, b2, b3;
    if (sscanf(line, "%*[^:]: %2x %2x %2x", &b1, &b2, &b3) == 3) {
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
  read_wav("input.wav", audioSamples, sampleRate, numChannels);

  std::vector<int16_t> audioOutput;

  // Process audio samples
  for (size_t i = 0; i < audioSamples.size(); i += numChannels) {
    if (numChannels == 1) {
      state.audioInL = state.audioInR = audioSamples[i];
    } else if (numChannels == 2) {
      state.audioInL = audioSamples[i];
      state.audioInR = audioSamples[i + 1];
    }

    state.runProgram();

    audioOutput.push_back(static_cast<int16_t>(state.audioOutL));
    audioOutput.push_back(static_cast<int16_t>(state.audioOutR));
  }

  write_wav("output.wav", audioOutput, sampleRate, 2);

  // for (size_t i = 0; i < 33; i++)
  //   state.runProgram();

  // for (size_t i = 0; i < 0x7f; i++) {
  //   printf("%04x: %06x\n", i, state.iram[i]);
  // }

  return 0;
}
