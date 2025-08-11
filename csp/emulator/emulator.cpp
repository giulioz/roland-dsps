#include "emulator.h"
#include "../../lsp/emulator/wavutils.h"
#include <cstdint>

int main() {
  Emulator *emulator1 = new Emulator();
  Emulator *emulator2 = new Emulator();
  emulator1->clear();
  emulator2->clear();
  bool useSecond = true;

  // FILE *pgmFile = fopen("sinetest.txt", "r");
  // FILE *pgmFile = fopen("srv_rev_test.txt", "r");
  // FILE *pgmFile = fopen("test.txt", "r");
  // FILE *pgmFile = fopen("noisetest.txt", "r");
  // FILE *pgmFile = fopen("dramlooptest.txt", "r");
  // FILE *pgmFile = fopen("../algos/sde_stereodelay.txt", "r");
  // FILE *pgmFile = fopen("../algos/sde_precisiondly.txt", "r");
  // FILE *pgmFile = fopen("../algos/sde_simpledelay.txt", "r");
  FILE *pgmFile = fopen("../algos/srv_chorusreverb.txt", "r");
  // FILE *pgmFile = fopen("../algos/se70_hall.txt", "r");
  // FILE *pgmFile = fopen("../algos/se70_stpitchshift.txt", "r");
  // FILE *pgmFile = fopen("../algos/se70_stchorus.txt", "r");
  // FILE *pgmFile = fopen("../algos/se70_stphaser.txt", "r");
  // FILE *pgmFile = fopen("../algos/se70_rotarymulti.txt", "r");
  // FILE *pgmFile = fopen("../algos/se70_simpledelay.txt", "r");
  // FILE *pgmFile = fopen("../algos/se70_metronome.txt", "r");
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
      emulator1->coefs[pos] = sign_extend<16>(param);
      emulator1->instr0[pos] = b0;
      emulator1->instr1[pos] = b1;
      emulator1->instr2[pos] = b2;
    } else if (pos >= 1024 && pos < 2048) {
      emulator2->coefs[pos - 1024] = sign_extend<16>(param);
      emulator2->instr0[pos - 1024] = b0;
      emulator2->instr1[pos - 1024] = b1;
      emulator2->instr2[pos - 1024] = b2;
    }

    pos++;
  }
  fclose(pgmFile);

  std::vector<int16_t> audioSamples;
  int sampleRate = 0;
  int nch = 0;
  // read_wav("../../lsp/emulator/input.wav", audioSamples, sampleRate, nch);
  read_wav("../../lsp/emulator/input_guit.wav", audioSamples, sampleRate, nch);
  // read_wav("../../lsp/emulator/sine.wav", audioSamples, sampleRate, nch);

  std::vector<int16_t> audioOutput;

  // SDE/SRV
  int IN_L = 3;
  int IN_R = 23;
  int OUT_L = 0;
  int OUT_R = 20;

  // SE70 32khz
  // int IN_L = 2;
  // int IN_R = 18;
  // int OUT_L = 11;
  // int OUT_R = 27;

  // SE70 48khz
  // int IN_L = 28;
  // int IN_R = 18;
  // int OUT_L = 2;
  // int OUT_R = 24;

  // Process audio samples
  for (size_t i = 0; i < audioSamples.size(); i += nch) {
    if (nch == 1) {
      emulator1->sioInput[IN_L] = emulator1->sioInput[IN_R] = audioSamples[i];
    } else if (nch == 2) {
      emulator1->sioInput[IN_L] = audioSamples[i];
      emulator1->sioInput[IN_R] = audioSamples[i + 1];
    }

    emulator1->sioInput[IN_L] <<= 6;
    emulator1->sioInput[IN_R] <<= 6;
    // emulator1->sioInput[IN_L] = 0;
    // emulator1->sioInput[IN_R] = 0;

    emulator1->runProgram();

    if (useSecond) {
      for (size_t i = 0; i < 32; i++) {
        emulator2->sioInput[i] = emulator1->sioOutput[i];
      }
      emulator2->runProgram();

      emulator2->sioOutput[OUT_L] >>= 8;
      emulator2->sioOutput[OUT_R] >>= 8;

      audioOutput.push_back(static_cast<int16_t>(emulator2->sioOutput[OUT_L]));
      audioOutput.push_back(static_cast<int16_t>(emulator2->sioOutput[OUT_R]));
    } else {
      emulator1->sioOutput[OUT_L] >>= 8;
      emulator1->sioOutput[OUT_R] >>= 8;

      audioOutput.push_back(static_cast<int16_t>(emulator1->sioOutput[OUT_L]));
      audioOutput.push_back(static_cast<int16_t>(emulator1->sioOutput[OUT_R]));
    }
  }

  write_wav("output.wav", audioOutput, sampleRate, 2);

  return 0;
}
