#include "emulator.h"
#include "../../lsp/emulator/wavutils.h"
#include <cstdint>

int main() {
  Emulator *emulator = new Emulator();
  emulator->clear();

  // FILE *pgmFile = fopen("sinetest.txt", "r");
  // FILE *pgmFile = fopen("test.txt", "r");
  FILE *pgmFile = fopen("noisetest.txt", "r");
  // FILE *pgmFile = fopen("../algos/sde_stereodelay.txt", "r");
  // FILE *pgmFile = fopen("../algos/sde_quaddelay.txt", "r");
  // FILE *pgmFile = fopen("../algos/se70_hall.txt", "r");
  // FILE *pgmFile = fopen("../algos/se70_stphaser.txt", "r");
  // FILE *pgmFile = fopen("../algos/se70_metronome.txt", "r");
  // FILE *pgmFile =
  // fopen("/Users/giuliozausa/personal/programming/Nuked-SC55/build/csp.txt",
  // "r");
  if (!pgmFile) {
    fprintf(stderr, "Error opening program file.\n");
    return 1;
  }
  char line[512] = {0};
  int pos = 0;
  while (fgets(line, sizeof(line), pgmFile)) {
    uint32_t addr;
    uint8_t b0, b1, b2;
    uint8_t dram_ctrl, mac_shift, opcode, store;
    uint16_t ram_offs, param;

    // sscanf(line, "%hhx %hhx %hhx %hx", &b0, &b1, &b2, &param);

    int n = sscanf(line,
                   "%x: %hhx %hhx %hhx dram_ctrl:%hhx mac_shift:%hhx "
                   "opcode:%hhx store:%hhx ram_offs:%hx param:%hx",
                   &addr, &b0, &b1, &b2, &dram_ctrl, &mac_shift, &opcode,
                   &store, &ram_offs, &param);

    if (n != 10) {
      fprintf(stderr, "Error parsing line: %s\n", line);
      continue;
    }

    if (pos < 1024) {
      emulator->coefs[pos] = sign_extend<16>(param);
      emulator->instr0[pos] = b0;
      emulator->instr1[pos] = b1;
      emulator->instr2[pos] = b2;
    }

    pos++;
  }
  fclose(pgmFile);

  std::vector<int16_t> audioSamples;
  int sampleRate = 0;
  int numChannels = 0;
  read_wav("../../lsp/emulator/input_guit.wav", audioSamples, sampleRate,
           numChannels);

  std::vector<int16_t> audioOutput;

  // SDE
  int IN_L = 3;
  int IN_R = 23;
  int OUT_L = 0;
  int OUT_R = 20;

  // SE70
  // int IN_L = 2;
  // int IN_R = 18;
  // int OUT_L = 11;
  // int OUT_R = 27;

  // Process audio samples
  for (size_t i = 0; i < audioSamples.size(); i += numChannels) {
    if (numChannels == 1) {
      emulator->sioInput[IN_L] = emulator->sioInput[IN_R] = audioSamples[i];
    } else if (numChannels == 2) {
      emulator->sioInput[IN_L] = audioSamples[i];
      emulator->sioInput[IN_R] = audioSamples[i + 1];
    }

    emulator->sioInput[IN_L] <<= 6;
    emulator->sioInput[IN_R] <<= 6;
    
    emulator->runProgram();

    // printf("L: %08x, R: %08x\n", emulator->sioOutput[OUT_L],
    //        emulator->sioOutput[OUT_R]);

    emulator->sioOutput[OUT_L] >>= 8;
    emulator->sioOutput[OUT_R] >>= 8;

    audioOutput.push_back(static_cast<int16_t>(emulator->sioOutput[OUT_L]));
    audioOutput.push_back(static_cast<int16_t>(emulator->sioOutput[OUT_R]));
  }

  write_wav("output.wav", audioOutput, sampleRate, 2);

  return 0;
}
