#include <chrono>
#include <cstdint>
#include <fstream>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "../sc88_pgmgen/mcu.h"
#include "codegen.h"
#include "emulator.h"
#include "runtime.h"

LspState lsp;
Runtime runtime;

char hexCharToInt(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}

int main(int argc, char *argv[]) {
  MCU mcu;
  memset(&mcu.mcu, 0, sizeof(MCU::mcu_core));

  mcu.MCU_Reset();
  FILE *state_file = fopen("../sc88_pgmgen/state_loadlsp.bin", "rb");
  uint8_t state_data[76145];
  fread(state_data, 1, sizeof(state_data), state_file);
  mcu.loadState(state_data);

  FILE *rom2_file = fopen("../sc88_pgmgen/sc88pro_valid.bin", "rb");
  fread(mcu.rom2, 1, ROM2_SIZE, rom2_file);
  fclose(rom2_file);

  std::string str =
      std::string("0120002410407f7f000000000000000000000040400068");
  uint8_t all_params[23] = {0};
  for (int i = 0; i < str.length() && i < 23 * 2; i += 2) {
    if (i + 1 < str.length()) {
      auto subs = str.substr(i, i + 2);
      all_params[i / 2] = (uint8_t)hexCharToInt(subs.c_str()[0]) * 16 +
                          (uint8_t)hexCharToInt(subs.c_str()[1]);
      // printf("%02x ", all_params[i / 2]);
    }
  }

  for (size_t i = 0; i < 0x80; i++) {
    all_params[6] = i;

    mcu.rom2[0xd0100 + 0x3a] = all_params[0]; // mfx group
    mcu.rom2[0xd0100 + 0x3b] = all_params[1]; // mfx type
    for (size_t i = 0; i < 0x15; i++) {
      mcu.rom2[0xd0100 + 0x3c + i] = all_params[2 + i]; // mfx params
    }

    mcu.MCU_Reset();
    mcu.loadState(state_data);
    mcu.MCU_Run();

    memcpy(lsp.iram, mcu.lsp_temp, sizeof(mcu.lsp_temp));
    lsp.iram[0x081] |= 0x7f0000;
    lsp.iram[0x1f3] |= 0x7f0000;
    lsp.parseProgram(true);

    std::vector<std::string> initLines;

    // printf("%02x: %06x %06x\n", i, mcu.lsp_temp[0x0b0], mcu.lsp_temp[0x0b1]);
    printf("%02x: %s %s\n", i, getCodeLine(lsp.instrCache[0x08c-0x80], initLines, false).c_str(), getCodeLine(lsp.instrCache[0x09b-0x80], initLines, false).c_str());
  }

  FILE *f = fopen("lsp_dump.bin", "wb");
  fwrite(mcu.lsp_temp + 0x80, 1, sizeof(mcu.lsp_temp) - (0x80 * 4), f);
  fclose(f);

  FILE *outputFile = fopen("compiled.cpp", "w");
  std::string code = convertToCode(lsp.instrCache);
  fprintf(outputFile, "%s", code.c_str());
  fclose(outputFile);

  return 0;
}
