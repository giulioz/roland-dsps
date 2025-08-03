#include "../../lsp/emulator/wavutils.h"
#include <cstdint>

static constexpr int DATA_BITS = 24;
static constexpr int64_t MIN_VAL = -(1LL << (DATA_BITS - 1));
static constexpr int64_t MAX_VAL = (1LL << (DATA_BITS - 1)) - 1;
static constexpr int32_t clamp_24(int64_t v) {
  if (v > MAX_VAL)
    return static_cast<int32_t>(MAX_VAL);
  if (v < MIN_VAL)
    return static_cast<int32_t>(MIN_VAL);
  return static_cast<int32_t>(v);
}
static constexpr int32_t sign_extend_24(int32_t x) {
  x &= 0xffffff;
  if (x & 0x800000) // If sign bit is set
    x |= ~0xffffff;
  return x;
}
static constexpr int64_t arshift_round(int64_t x, int64_t k) {
  int64_t rnd = 1 << (k - 3);
  return (x - rnd) >> k;
}

static constexpr int pipelineWriteDelay = 3;

class DspAccumulator {
public:
  int32_t acc = 0;
  int32_t hist[4] = {0};
  std::size_t head = 0;

  inline void set(int32_t v) { acc = v; }
  inline void add(int32_t v) { acc += v; }

  inline int32_t operator=(int32_t v) {
    acc = v;
    return acc;
  }

  inline int32_t operator+=(int32_t v) {
    acc += v;
    return acc;
  }

  inline void abs() {
    if (acc < 0)
      acc = -acc;
  }

  bool pos() { return acc >= 0; }

  inline int32_t sat24() const { return clamp_24(acc); }
  inline int32_t raw24() const { return sign_extend_24(acc & 0xffffff); }

  inline int32_t historySat24(std::size_t n) const {
    std::size_t idx = (head - n) & 3;
    return clamp_24(hist[idx]);
  }
  inline int32_t historyRaw24(std::size_t n) const {
    std::size_t idx = (head - n) & 3;
    return sign_extend_24(hist[idx] & 0xffffff);
  }
  inline int32_t historyRaw32(std::size_t n) const {
    std::size_t idx = (head - n) & 3;
    return hist[idx];
  }

  inline void fwdPipeline() { head = (head + 1) & 3; }
  inline void storePipeline() { hist[head] = acc; }
};

#define ERAM_SIZE 0x40000

#define ACC_SIZE (int64_t)38
#define ACC_MASK (int64_t)(((int64_t)1 << ACC_SIZE) - (int64_t)1)

class Emulator {
public:
  int32_t sioInput[32] = {0};
  int32_t sioOutput[32] = {0};

  int32_t coefs[1024] = {0};
  uint8_t instr0[1024] = {0};
  uint8_t instr1[1024] = {0};
  uint8_t instr2[1024] = {0};

  int32_t iram[512] = {0};
  int32_t eram[ERAM_SIZE] = {0};

  uint16_t pc = 0;
  uint8_t iramPos = 0;
  uint32_t eramPos = 0;

  uint8_t eramMode = 0;
  uint16_t eramPCStart = 0;
  uint32_t eramImmOffset = 0;
  uint32_t eramVarOffset = 0;
  int32_t eramWriteLatch = 0;
  int32_t eramReadLatch = 0;

  bool inputFromSpecial = false;
  int64_t opcodeInput = 0;
  int32_t mulCoefA = 0;
  int32_t mulCoefB = 0;

  DspAccumulator accA, accB;

  void writePgm(uint32_t pos, uint8_t b0, uint8_t b1, uint8_t b2,
                int32_t param) {
    instr0[pos] = b0;
    instr1[pos] = b1;
    instr2[pos] = b2;
    coefs[pos] = param;
  }

  void runProgram() {
    for (pc = 0; pc < 1024; pc++) {
      accA.fwdPipeline();
      accB.fwdPipeline();

      inputFromSpecial = false;

      doEram();
      doStore();

      if (!inputFromSpecial) {
        uint16_t memOffs = instr2[pc] | (instr1[pc] & 1) << 8;
        opcodeInput =
            (int64_t)iram[(iramPos + memOffs) & 0x1ff] * (int64_t)coefs[pc];
        if (memOffs == 1) {
          opcodeInput = coefs[pc] << 15;
        } else if (memOffs == 2) {
          opcodeInput = coefs[pc] << (15 + 7);
        }
      } else {
        opcodeInput <<= 15;
      }

      doOpcode();

      accA.storePipeline();
      accB.storePipeline();
    }

    iramPos = (iramPos - 1) & 0x1ff;
    eramPos -= 1;
  }

  void doEram() {
    uint8_t eramCtrl = instr0[pc] & 0xf8;
    uint8_t stage = pc - eramPCStart;

    // start eram command
    if ((eramCtrl & 0x8) != 0) {
      if (stage <= 6) {
        // printf("error: ERAM command started at stage %d\n", stage);
      }

      eramMode = eramCtrl & 0xc0;
      eramPCStart = pc;
      eramImmOffset = 0;
    }

    // accumulate immediates
    uint8_t eramAdj = eramCtrl >> 4;
    if (eramAdj != 0 && stage <= 5) {
      eramImmOffset += eramAdj << ((stage - 1) * 4);
    }

    // addr computation
    uint32_t effectiveAddr = (eramPos + eramImmOffset) & (ERAM_SIZE - 1);
    if (eramMode == 0x48) {
      effectiveAddr =
          ((eramImmOffset & 1) + (eramVarOffset >> 10)) & (ERAM_SIZE - 1);
    } else if (eramMode == 0xc0) {
      effectiveAddr = (eramPos + (eramImmOffset & 1) + (eramVarOffset >> 10)) &
                      (ERAM_SIZE - 1);
    }

    // write
    if (stage == 6 && eramMode == 0x40) {
      eram[effectiveAddr] = eramWriteLatch;
    }

    // read
    else if (stage == 6) {
      eramReadLatch = eram[effectiveAddr];
    }
  }

  void doStore() {
    uint8_t storeCtrl = instr1[pc] & 0xe;
    uint16_t memOffs = instr2[pc] | (instr1[pc] & 1) << 8;

    if (storeCtrl == 0x02) {
      // printf("error: store to 0x%04x with ctrl %02x\n", memOffs, storeCtrl);
      opcodeInput = 0x800000;
      inputFromSpecial = true;
    }

    else if (storeCtrl == 0x04) {
      opcodeInput = 0x000000;
      inputFromSpecial = true;
      doSpecial(memOffs, false);
    }

    else if (storeCtrl == 0x06) {
      opcodeInput = 0x000000;
      inputFromSpecial = true;
      doSpecial(memOffs, true);
    }

    else if (storeCtrl == 0x08) {
      iram[(memOffs + iramPos) & 0x1ff] = accA.historySat24(pipelineWriteDelay);
    }

    else if (storeCtrl == 0x0a) {
      iram[(memOffs + iramPos) & 0x1ff] = accB.historyRaw24(pipelineWriteDelay);
    }

    else if (storeCtrl == 0x0c) {
      iram[(memOffs + iramPos) & 0x1ff] = accA.historySat24(pipelineWriteDelay);
    }

    else if (storeCtrl == 0x0e) {
      iram[(memOffs + iramPos) & 0x1ff] = accB.historyRaw24(pipelineWriteDelay);
    }
  }

  void doSpecial(uint16_t specialId, bool saturated) {
    // Serial Input
    if (specialId >= 0x190 && specialId <= 0x1af) {
      opcodeInput = sioInput[specialId - 0x190];
      // printf("input at %d %d\n", specialId - 0x190, opcodeInput);
    }

    // Serial Output accA
    else if (specialId >= 0x1b0 && specialId <= 0x1cf) {
      if (saturated) {
        sioOutput[specialId - 0x1b0] = accA.historySat24(pipelineWriteDelay);
        // printf("output at %d %d\n", specialId - 0x1b0,
        // accA.historySat24(pipelineWriteDelay));
      } else {
        sioOutput[specialId - 0x1b0] = accA.historyRaw24(pipelineWriteDelay);
        // printf("output at %d %d\n", specialId - 0x1b0,
        // accA.historyRaw24(pipelineWriteDelay));
      }
    }

    // Serial Output accB
    else if (specialId >= 0x1d0 && specialId <= 0x1ef) {
      if (saturated) {
        sioOutput[specialId - 0x1d0] = accB.historySat24(pipelineWriteDelay);
        // printf("output at %d %d\n", specialId - 0x1d0,
        // accB.historySat24(pipelineWriteDelay));
      } else {
        sioOutput[specialId - 0x1d0] = accB.historyRaw24(pipelineWriteDelay);
        // printf("output at %d %d\n", specialId - 0x1d0,
        // accB.historyRaw24(pipelineWriteDelay));
      }
    }

    // ERAM tap
    else if (specialId >= 0x1f0 && specialId <= 0x1ff) {
      opcodeInput = eramReadLatch;
    }

    // Branching
    else if (specialId >= 0x172 && specialId <= 0x177) {
      uint16_t jmpDest = coefs[pc];
      bool shouldJump = true;

      if (specialId == 0x172) { // JMP
        shouldJump = true;
      } else if (specialId == 0x173) { // JMP accA > 0
        shouldJump = accA.historyRaw32(pipelineWriteDelay) > 0;
      } else if (specialId == 0x174) { // JMP accA sat overflow
        shouldJump = accA.historyRaw32(pipelineWriteDelay) > 0x7fffff ||
                     accA.historyRaw32(pipelineWriteDelay) < -0x800000;
      } else if (specialId == 0x175) { // JMP accA < 0
        shouldJump = accA.historyRaw32(pipelineWriteDelay) < 0;
      } else if (specialId == 0x176) { // JMP accA == 0
        shouldJump = accA.historyRaw32(pipelineWriteDelay) == 0;
      } else if (specialId == 0x177) { // JMP accA >= 0
        shouldJump = accA.historyRaw32(pipelineWriteDelay) >= 0;
      }

      if (shouldJump) {
        pc = jmpDest -
             1; // -1 because pc will be incremented at the end of the loop
      }
    }

    // Host interface
    else if (specialId == 0x182) {
      // printf("Host interface write: %06x\n",
      //        accA.historyRaw32(pipelineWriteDelay));
    } else if (specialId == 0x18a) {
      // printf("Host interface write: %06x\n",
      //        accB.historyRaw32(pipelineWriteDelay));
    }

    // ERAM write latch
    else if (specialId == 0x183) {
      eramWriteLatch = accA.historyRaw32(pipelineWriteDelay);
    } else if (specialId == 0x18b) {
      eramWriteLatch = accB.historyRaw32(pipelineWriteDelay);
    }

    // ERAM second tap pos
    else if (specialId == 0x185) {
      eramVarOffset = accA.historyRaw32(pipelineWriteDelay);
      mulCoefA = (eramVarOffset & 0x3ff) << 13;
      mulCoefB = eramVarOffset;
    } else if (specialId == 0x18d) {
      eramVarOffset = accB.historyRaw32(pipelineWriteDelay);
      mulCoefA = (eramVarOffset & 0x3ff) << 13;
      mulCoefB = eramVarOffset;
    }

    // Mult coefficients
    else if (specialId == 0x186) {
      mulCoefA = accA.historyRaw32(pipelineWriteDelay);
    } else if (specialId == 0x18e) {
      mulCoefA = accB.historyRaw32(pipelineWriteDelay);
    } else if (specialId == 0x187) {
      mulCoefB = accA.historyRaw32(pipelineWriteDelay);
    } else if (specialId == 0x18f) {
      mulCoefB = accB.historyRaw32(pipelineWriteDelay);
    }

    else {
      printf("error: invalid special ID %04x\n", specialId);
    }
  }

  void doOpcode() {
    uint8_t opcode = instr1[pc] & 0xf0;

    uint8_t scaler = 15;
    uint8_t scalerCtrl = instr0[pc] & 3;
    if (scalerCtrl == 1) {
      scaler = 14;
    } else if (scalerCtrl == 2) {
      scaler = 13;
    } else if (scalerCtrl == 3) {
      scaler = 11;
    }

    // MAC
    if (opcode == 0x00) {
      accA += (opcodeInput >> scaler);
    } else if (opcode == 0x10) {
      accB += (opcodeInput >> scaler);
    } else if (opcode == 0x20) {
      accA = (opcodeInput >> scaler);
    } else if (opcode == 0x30) {
      accB = (opcodeInput >> scaler);
    }

    // ABS_CLAMP
    else if (opcode == 0x40 || opcode == 0x50) {
      if (opcodeInput < 0) {
        opcodeInput = ~opcodeInput;
      }
      opcodeInput &= ACC_MASK;
      opcodeInput >>= scaler;

      if (opcode == 0x40) {
        accA = opcodeInput;
      } else if (opcode == 0x50) {
        accA += opcodeInput;
      }

      // TODO this clamp is only done when storing to iram?
      if (accA.pos()) {
        accA = 0;
      }
    }

    // NEG
    else if (opcode == 0x60 || opcode == 0x70) {
      if (opcodeInput >= 0) {
        opcodeInput = ~opcodeInput;
      }
      opcodeInput &= ACC_MASK;
      opcodeInput >>= scaler;

      if (opcode == 0x60) {
        accA = opcodeInput;
      } else if (opcode == 0x70) {
        accA += opcodeInput;
      }
    }

    // ABS
    else if (opcode == 0x80 || opcode == 0x90) {
      if (opcodeInput < 0) {
        opcodeInput = ~opcodeInput;
      }
      opcodeInput &= ACC_MASK;
      opcodeInput >>= scaler;

      if (opcode == 0x80) {
        accA = opcodeInput;
      } else if (opcode == 0x90) {
        accA += opcodeInput;
      }
    }

    // MAC_LOW
    else if (opcode == 0xa0) {
      accA += (opcodeInput >> 15) >> scaler;
    } else if (opcode == 0xb0) {
      accB += (opcodeInput >> 15) >> scaler;
    }

    // MUL
    else if (opcode == 0xc0 || opcode == 0xd0 || opcode == 0xe0 ||
             opcode == 0xf0) {
      int16_t coef = coefs[pc];
      uint8_t mem_offs = instr2[pc] | (instr1[pc] & 1) << 8;

      int64_t mem_val = opcodeInput;

      if (mem_offs == 1) {
        mem_val = 0x8000;
      } else if (mem_offs == 2) {
        mem_val = 0x8000 << 7;
      }

      int64_t m = sign_extend_24(mulCoefA);
      if ((coef & 1) != 0) {
        m = sign_extend_24(mulCoefB);
      }

      int64_t mul_val = 0;
      if (coef == 0x0 or coef == 0x1) {
        // TODO
      } else if (coef == 0x2 or coef == 0x3) {
        // TODO
        int64_t mul = (~mem_val * (~(m & 0xffffff) >> 8));
        int64_t mul_s = ~arshift_round(mul, 15);
        mul_val = mem_val + mul_s;
      } else if (coef == 0x4 or coef == 0x5) {
        mul_val = (mem_val * ~(m >> 8)) >> scaler;
      } else if (coef == 0x6 or coef == 0x7) {
        mul_val = (mem_val * (m >> 8)) >> scaler;
      }

      if (opcode == 0xc) {
        accA = mul_val;
      } else if (opcode == 0xd) {
        accB = mul_val;
      } else if (opcode == 0xe) {
        accA += mul_val;
      } else if (opcode == 0xf) {
        accB += mul_val;
      }
    }
  }
};

int main() {
  Emulator emulator;

  // FILE *pgmFile = fopen("../algos/sde_simpledelay.txt", "r");
  FILE *pgmFile = fopen("../algos/se70_metronome.txt", "r");
  // FILE *pgmFile = fopen("../algos/se70_test.txt", "r");
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

    emulator.coefs[pos] = param;
    emulator.instr0[pos] = b0;
    emulator.instr1[pos] = b1;
    emulator.instr2[pos] = b2;

    pos++;
  }
  fclose(pgmFile);

  // emulator.writePgm(0, 0x01, 0x27, 0x93, 0x4000);
  // emulator.writePgm(1, 0x00, 0x00, 0x00, 0x0000);
  // emulator.writePgm(2, 0x00, 0x00, 0x00, 0x0000);
  // emulator.writePgm(3, 0x00, 0x0c, 0x10, 0x0000);
  // emulator.writePgm(4, 0x00, 0x37, 0xa7, 0x4000);
  // emulator.writePgm(5, 0x00, 0x00, 0x00, 0x0000);
  // emulator.writePgm(6, 0x00, 0x00, 0x00, 0x0000);
  // emulator.writePgm(7, 0x00, 0x0e, 0x11, 0x0000);

  // emulator.writePgm(8, 0x01, 0x20, 0x10, 0x4000);
  // emulator.writePgm(9, 0x00, 0x00, 0x00, 0x0000);
  // emulator.writePgm(10, 0x00, 0x00, 0x00, 0x0000);
  // emulator.writePgm(11, 0x00, 0x07, 0xb0, 0x0000);
  // emulator.writePgm(12, 0x00, 0x30, 0x11, 0x4000);
  // emulator.writePgm(13, 0x00, 0x00, 0x00, 0x0000);
  // emulator.writePgm(14, 0x00, 0x00, 0x00, 0x0000);
  // emulator.writePgm(15, 0x00, 0x07, 0xe4, 0x0000);

  std::vector<int16_t> audioSamples;
  int sampleRate = 0;
  int numChannels = 0;
  read_wav("../../lsp/emulator/input_guit.wav", audioSamples, sampleRate,
           numChannels);

  std::vector<int16_t> audioOutput;

  // #define IN_L 3
  // #define IN_R 23
  // #define OUT_L 0
  // #define OUT_R 20

#define IN_L 2
#define IN_R 18
#define OUT_L 11
#define OUT_R 27

  // Process audio samples
  for (size_t i = 0; i < audioSamples.size(); i += numChannels) {
    if (numChannels == 1) {
      emulator.sioInput[IN_L] = emulator.sioInput[IN_R] = audioSamples[i];
    } else if (numChannels == 2) {
      emulator.sioInput[IN_L] = audioSamples[i];
      emulator.sioInput[IN_R] = audioSamples[i + 1];
    }

    emulator.sioInput[IN_L] <<= 6;
    emulator.sioInput[IN_R] <<= 6;

    emulator.runProgram();

    emulator.sioOutput[OUT_L] >>= 8;
    emulator.sioOutput[OUT_R] >>= 8;

    audioOutput.push_back(static_cast<int16_t>(emulator.sioOutput[OUT_L]));
    audioOutput.push_back(static_cast<int16_t>(emulator.sioOutput[OUT_R]));
  }

  write_wav("output.wav", audioOutput, sampleRate, 2);

  return 0;
}
