#include <cstdint>
#include <fstream>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <string>

// Mask to 24 bits and sign-extend
int32_t sign_extend_24(int32_t x) {
  x &= 0xffffff;
  if (x & 0x800000) // If sign bit is set
    x |= ~0xffffff;
  return x;
}

// Performs signed 24-bit addition with saturation.
// Inputs are assumed to be signed 24-bit values stored in int32_t.
// Returns the saturated signed 24-bit result as int32_t.
int32_t add24_sat(int32_t a, int32_t b) {

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
    if (!(opcode == 0xc0 && memOffset == 0x58)) {
      if (writeCtrl == 0x08) {
        iram[ramPos] = getAccAForStore();
      } else if (writeCtrl == 0x10) {
        iram[ramPos] = getAccBForStore();
      } else if (writeCtrl == 0x18) {
        // TODO: no saturation
        iram[ramPos] = getAccAForStore();
      }
    }

    // Audio output
    if (ii == 0xc8 && memOffset == 0x58) {
      audioOut = getAccAForStore();
    }

    // Multiply
    int32_t multA = iram[ramPos];
    int32_t multB = cc;
    if (opcode == 0xc0) {
      if (memOffset == 0x50) {
        // TODO: when using this mode, only the immediately previous accumulator
        // should work
        multA = accA >> 6;
        multB = (uint8_t)cc;
      } else if (memOffset == 0x7e || memOffset == 0x7f) {
        multA = audioIn;
      }
    }
    if (opcode == 0xe0) {
      if (memOffset == 0x50) {
        // TODO: when using this mode, only the immediately previous accumulator
        // should work
        multA = accB >> 6;
        multB = (uint8_t)cc;
      } else if (memOffset == 0x7e || memOffset == 0x7f) {
        multA = audioIn;
      }
    }
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
      if (memOffset == 0x50)
        accA = add24_sat(accA, multRes);
      else if (memOffset != 0x58)
        accA = add24_sat(0, multRes);
    } else if (opcode == 0xe0) {
      // ??
      if (rr == 0x50)
        accB = add24_sat(accB, cc >> 3);
      else if (rr != 0xd0)
        accB = add24_sat(accB, cc >> 1);
      else if (memOffset != 0x58)
        accB = add24_sat(0, multRes);
    } else {
      printf("Unknown opcode: %02x\n", opcode);
    }
  }
};
