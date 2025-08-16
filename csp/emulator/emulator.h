#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static constexpr int64_t ERAM_SIZE = 0x10000;       // SE-70
static constexpr int64_t ERAM_DATA_MASK = 0xfffff0; // SE-70

// static constexpr int64_t ERAM_SIZE = 0x20000;       // SDE/SRV
// static constexpr int64_t ERAM_DATA_MASK = 0xffffff; // SDE/SRV

static constexpr int64_t PRAM_SIZE = 0x400;
static constexpr int64_t IRAM_SIZE = 0x200;
static constexpr int64_t IRAM_MASK = IRAM_SIZE - 1;
static constexpr int64_t ERAM_SIZE_FULL = 0x40000;
static constexpr int64_t ERAM_MASK = ERAM_SIZE - 1;
static constexpr int64_t ERAM_MASK_FULL = ERAM_SIZE_FULL - 1;

static constexpr int64_t ERAM_COMMIT_STAGE = 10;

static constexpr int64_t ACC_BITS = 30;
static constexpr int64_t MUL_BITS_A = 30;
static constexpr int64_t MUL_BITS_B = 16;
static constexpr int64_t MUL_BITS_R = 38;
static constexpr int64_t MUL_MASK_A = (((int64_t)1 << MUL_BITS_A) - 1);
static constexpr int64_t MUL_MASK_B = (((int64_t)1 << MUL_BITS_B) - 1);
static constexpr int64_t MUL_MASK_R = (((int64_t)1 << (MUL_BITS_R)) - 1);

static constexpr int PIPELINE_WRITE_DELAY = 3;

template <int32_t n_bits> static constexpr int32_t clamp_bits(int32_t v) {
  if (v > (1LL << (n_bits - 1)) - 1)
    return static_cast<int32_t>((1LL << (n_bits - 1)) - 1);
  if (v < -(1LL << (n_bits - 1)))
    return static_cast<int32_t>(-(1LL << (n_bits - 1)));
  return static_cast<int32_t>(v);
}

template <int32_t n_bits> static constexpr int32_t sign_extend(int32_t x) {
  x &= ((1 << n_bits) - 1);
  if (x & (1 << (n_bits - 1))) // If sign bit is set
    x |= ~((1 << n_bits) - 1);
  return x;
}

class DspAccumulator {
public:
  int32_t acc = 0;
  int32_t hist[4] = {0};
  size_t head = 0;

  inline void ensureBounds() { acc = sign_extend<ACC_BITS>(acc); }

  inline int32_t operator=(int32_t v) {
    acc = v;
    ensureBounds();
    return acc;
  }

  inline int32_t operator+=(int32_t v) {
    acc += v;
    ensureBounds();
    return acc;
  }

  inline int32_t sat24() const { return clamp_bits<24>(acc); }
  inline int32_t raw24() const { return sign_extend<24>(acc); }
  inline int32_t rawFull() const { return acc; }

  inline int32_t historySat24(size_t n) const {
    size_t idx = (head - n) & 3;
    return clamp_bits<24>(hist[idx]);
  }
  inline int32_t historyRaw24(size_t n) const {
    size_t idx = (head - n) & 3;
    return sign_extend<24>(hist[idx]);
  }
  inline int32_t historyRawFull(size_t n) const {
    size_t idx = (head - n) & 3;
    return hist[idx];
  }

  inline void fwdPipeline() { head = (head + 1) & 3; }
  inline void storePipeline() { hist[head] = acc; }
};

class Emulator {
public:
  int32_t sioInput[32] = {0};
  int32_t sioOutput[32] = {0};

  int32_t coefs[PRAM_SIZE] = {0};
  uint8_t instr0[PRAM_SIZE] = {0};
  uint8_t instr1[PRAM_SIZE] = {0};
  uint8_t instr2[PRAM_SIZE] = {0};

  int32_t iram[IRAM_SIZE] = {0};
  int32_t eram[ERAM_SIZE_FULL] = {0};

  uint16_t pc = 0;
  uint16_t iramPos = 0;
  uint32_t eramPos = 0;

  int opcode5Cycles = 0;
  int opcode5Sign = 0;

  bool eramActiveCurrent = false;
  uint8_t eramModeCurrent = 0;
  uint16_t eramPCStartCurrent = 0;
  uint32_t eramEffectiveAddr = 0;

  bool eramActiveNext = false;
  uint8_t eramModeNext = 0;
  uint16_t eramPCStartNext = 0;
  uint32_t eramImmOffsetAccNext = 0;

  uint32_t eramVarOffset = 0;
  int32_t eramWriteLatch = 0;
  int32_t eramReadLatch = 0;

  int32_t hostReadback = 0;
  int32_t mulCoefA = 0;
  int32_t mulCoefB = 0;

  DspAccumulator accA, accB;

  void clear() {
    memset(coefs, 0, sizeof(coefs));
    memset(instr0, 0, sizeof(instr0));
    memset(instr1, 0, sizeof(instr1));
    memset(instr2, 0, sizeof(instr2));
    memset(iram, 0, sizeof(iram));
    memset(eram, 0, sizeof(eram));
    memset(sioInput, 0, sizeof(sioInput));
    memset(sioOutput, 0, sizeof(sioOutput));
    pc = 0;
    iramPos = 0;
    eramPos = 0;
    opcode5Cycles = 0;
    opcode5Sign = 0;
    eramActiveCurrent = false;
    eramModeCurrent = 0;
    eramPCStartCurrent = 0;
    eramEffectiveAddr = 0;
    eramActiveNext = false;
    eramModeNext = 0;
    eramPCStartNext = 0;
    eramImmOffsetAccNext = 0;
    eramVarOffset = 0;
    eramWriteLatch = 0;
    eramReadLatch = 0;
    mulCoefA = 0;
    mulCoefB = 0;
    hostReadback = 0;
    accA = 0;
    accB = 0;
  }

  void writePgm(uint32_t pos, uint8_t b0, uint8_t b1, uint8_t b2,
                int32_t param) {
    instr0[pos] = b0;
    instr1[pos] = b1;
    instr2[pos] = b2;
    coefs[pos] = sign_extend<16>(param);
  }

  inline void writeIramOffset(uint16_t offset, int32_t value) {
    uint32_t ramPos = ((uint32_t)offset + iramPos) & IRAM_MASK;
    iram[ramPos] = value;
  }

  inline int32_t readIramOffset(uint16_t offset) {
    uint32_t ramPos = ((uint32_t)offset + iramPos) & IRAM_MASK;
    return iram[ramPos];
  }

  void runProgram() {
    int limit = 1024;
    for (pc = 0; pc < PRAM_SIZE; pc++) {
      limit--;
      if (limit == 0)
        break;

      accA.fwdPipeline();
      accB.fwdPipeline();

      doEram();

      if (opcode5Cycles > 0) {
        opcode5Cycles--;
      }

      // Decode instr
      uint8_t storeCtrl = instr1[pc] & 0xe;
      uint16_t memOffs = instr2[pc] | ((instr1[pc] & 1) << 8);
      uint8_t opcode = instr1[pc] & 0xf0;
      uint8_t scalerCtrl = instr0[pc] & 3;
      int16_t coef = coefs[pc];

      // Multiplier inputs
      int64_t mulInputA_24 = readIramOffset(memOffs);
      int64_t mulInputB_16 = coef;

      // Store
      bool storeSpecial = storeCtrl == 0x04 || storeCtrl == 0x06;
      bool storeIram = storeCtrl == 0x08 || storeCtrl == 0x0a ||
                       storeCtrl == 0x0c || storeCtrl == 0x0e;
      bool storeSaturate =
          storeCtrl == 0x06 || storeCtrl == 0x0c || storeCtrl == 0x0e;
      DspAccumulator *storeAcc =
          storeCtrl == 0x0a || storeCtrl == 0x0e ? &accB : &accA;

      if (storeCtrl == 0x02) {
        // Synth bus input
        mulInputA_24 = sign_extend<24>(0x800000); // SE-70
        writeIramOffset(memOffs, mulInputA_24);
      }

      else if (storeIram) {
        if (memOffs == 1 || memOffs == 2) {
          printf("store to immediate?\n");
        }

        if (storeSaturate) {
          mulInputA_24 = storeAcc->historySat24(PIPELINE_WRITE_DELAY);
        } else {
          mulInputA_24 = storeAcc->historyRaw24(PIPELINE_WRITE_DELAY);
        }

        writeIramOffset(memOffs, mulInputA_24);
      }

      else if (storeSpecial) {
        if (memOffs == 1 || memOffs == 2) {
          printf("store to immediate?\n");
        }

        doSpecialStore(memOffs, storeSaturate);

        // Serial Input
        if (memOffs >= 0x190 && memOffs <= 0x1af) {
          mulInputA_24 = sioInput[memOffs - 0x190];
        }

        // ERAM tap
        else if (memOffs >= 0x1f0 && memOffs <= 0x1ff) {
          writeIramOffset(memOffs, eramReadLatch);
          mulInputA_24 = eramReadLatch;
        }

        // else {
        //   if (storeSaturate) {
        //     mulInputA_24 = storeAcc->historySat24(PIPELINE_WRITE_DELAY);
        //   } else {
        //     mulInputA_24 = storeAcc->historyRaw24(PIPELINE_WRITE_DELAY);
        //   }
        // }
      }

      // Immediate load coef
      if (memOffs == 1) {
        mulInputA_24 = 0x008000;
      } else if (memOffs == 2) {
        mulInputA_24 = 0x400000;
      }

      if ((opcode5Cycles == 6 || opcode5Cycles == 5) && opcode5Sign == 1) {
        mulInputA_24 = 0;
      } else if ((opcode5Cycles > 0 && opcode5Cycles <= 4) && opcode5Sign == -1) {
        mulInputA_24 = 0;
      }

      // Multiplier mode
      bool mulShouldAbs4 = opcode == 0x40 || opcode == 0x50;
      bool mulShouldAbs8 = opcode == 0x80 || opcode == 0x90;
      bool mulForceNeg = opcode == 0x60 || opcode == 0x70;
      bool mulShouldClamp5 = opcode == 0x50;
      bool mulBFromVariable =
          opcode == 0xc0 || opcode == 0xd0 || opcode == 0xe0 || opcode == 0xf0;

      // Custom coef mult
      int64_t mulCoef = coef & 1 ? mulCoefB : mulCoefA;
      if (mulBFromVariable) {
        if (coef == 0x0001) {
          // TODO: dependent on host config reg 0x0808
          mulInputB_16 = 0x7ff0;
          // mulInputB_16 = 0x0;
        }

        else if (coef == 0x0002 || coef == 0x0003) {
          // mulInputB_16 = 0x8000 + ((~abs(mulCoef)) >> 8);
        }

        else if (coef == 0x004 || coef == 0x0005) {
          mulInputB_16 = ~(mulCoef >> 8);
        }

        else if (coef == 0x0006 || coef == 0x0007) {
          mulInputB_16 = mulCoef >> 8;
        }
      }

      // Multiplier
      uint8_t signA = (mulInputA_24 >> (MUL_BITS_A - 1)) & 1;
      uint8_t signB = (mulInputB_16 >> (MUL_BITS_B - 1)) & 1;
      uint8_t finalSign = signA ^ signB;
      int64_t mulInputA_24_abs = mulInputA_24;
      int64_t mulInputB_16_abs = mulInputB_16;
      if (signA) {
        mulInputA_24_abs = ~mulInputA_24_abs + 1;
      }
      if (signB) {
        mulInputB_16_abs = ~mulInputB_16_abs + 1;
      }
      int64_t mulResult = mulInputA_24_abs * mulInputB_16_abs;
      if (finalSign && !mulShouldAbs4) {
        mulResult = ~mulResult + 1;
      } else if (finalSign) {
        mulResult = mulResult - 1;
      }

      // Interp
      if (mulBFromVariable && (coef == 0x0002 || coef == 0x0003)) {
        mulInputB_16 = 0x8000 + ((~(mulCoef >= 0 ? mulCoef : ~mulCoef)) >> 8);
        if (mulCoef < 0) {
          mulInputB_16 += 1;
        }
        mulResult = mulInputA_24 * mulInputB_16;
        if (mulCoef < 0) {
          mulResult = -mulResult;
        }
      }

      // Abs
      if (mulShouldAbs8 && finalSign) {
        mulResult = ~mulResult;
      }

      // Neg
      if (mulForceNeg) {
        if (mulResult >= 0) {
          mulResult = ~mulResult;
        }
        mulResult &= MUL_MASK_R;
        if (mulInputA_24 == 0) {
          mulResult = signB ? -0x4000000000 : 0x3fffffffff;
        }
        if (mulInputB_16 == 0) {
          mulResult = signA ? -0x4000000000 : 0x3fffffffff;
        }
      }

      // Post multiplier scaler
      if (scalerCtrl == 0) {
        mulResult >>= 4;
      } else if (scalerCtrl == 1) {
        mulResult >>= 3;
      } else if (scalerCtrl == 2) {
        mulResult >>= 2;
      } else if (scalerCtrl == 3) {
        mulResult >>= 0;
      }
      mulResult >>= 11;

      // Accumulate inputs
      int64_t sumInA = 0;
      int64_t sumInB = mulResult;
      DspAccumulator *sumDest = &accA;

      if (opcode == 0x10 || opcode == 0x30 || opcode == 0xb0 ||
          opcode == 0xd0 || opcode == 0xf0) {
        sumDest = &accB;
      }

      if (opcode == 0x00 || opcode == 0x10 || opcode == 0x50 ||
          opcode == 0x70 || opcode == 0x90 || opcode == 0xa0 ||
          opcode == 0xb0 || opcode == 0xe0 || opcode == 0xf0) {
        sumInA = sumDest->rawFull();
      }

      if (opcode == 0xa0 || opcode == 0xb0) {
        sumInB >>= 15;
      }

      // Accumulate
      int64_t sumResult = sumInA + sumInB;

      // Clamp
      if (mulShouldClamp5) {
        opcode5Cycles = 9;
        opcode5Sign = sumResult < 0 ? -1 : 1;
      }

      (*sumDest) = sumResult;

      accA.storePipeline();
      accB.storePipeline();
    }

    iramPos = (iramPos - 1) & IRAM_MASK;
    eramPos = (eramPos - 1) & ERAM_MASK_FULL;
  }

  void doEram() {
    uint8_t eramCtrl = instr0[pc] & 0xf8;
    int stage1 = pc - eramPCStartNext;
    int stage2 = pc - eramPCStartCurrent;

    // Transaction start
    bool newStart = (eramCtrl & 0x8) != 0;
    if (newStart) {
      if (eramActiveNext) {
        printf("ERAM transaction already active at pc %03x\n", pc);
      }

      eramActiveNext = true;
      eramModeNext = eramCtrl >> 4;
      eramPCStartNext = pc;
      eramImmOffsetAccNext = 0;

      if (eramModeNext != (eramModeNext & 0xc)) {
        printf("wtf %03x\n", eramCtrl);
      }
    }

    // Accumulate immediates
    uint32_t eramAdj = eramCtrl >> 4;
    if (!newStart && eramActiveNext && stage1 <= 5 && stage1 > 0) {
      eramImmOffsetAccNext += eramAdj << ((stage1 - 1) << 2);
    }

    // Next stage
    if (eramActiveNext && stage1 == 5) {
      if (eramActiveCurrent) {
        printf("ERAM transaction already active at pc %03x\n", pc);
      }

      eramActiveCurrent = true;
      eramModeCurrent = eramModeNext;
      eramPCStartCurrent = eramPCStartNext;

      eramActiveNext = false;

      // Addr computation
      eramEffectiveAddr = eramPos + eramImmOffsetAccNext;
      if (eramModeNext == 0x8) {
        eramEffectiveAddr = eramVarOffset + (eramImmOffsetAccNext & 1);
      } else if (eramModeNext == 0xc) {
        eramEffectiveAddr =
            eramPos + eramVarOffset + (eramImmOffsetAccNext & 1);
      }
    }

    // Write
    if (eramActiveCurrent && stage2 == ERAM_COMMIT_STAGE &&
        eramModeCurrent == 0x4) {
      eramActiveCurrent = false;
      eram[eramEffectiveAddr & ERAM_MASK] = eramWriteLatch;
    }

    // Read
    else if (eramActiveCurrent && stage2 == ERAM_COMMIT_STAGE &&
             eramModeCurrent != 0x4) {
      eramActiveCurrent = false;
      eramReadLatch =
          sign_extend<24>(eram[eramEffectiveAddr & ERAM_MASK] & ERAM_DATA_MASK);
    }
  }

  void doSpecialStore(uint16_t specialId, bool saturated) {
    DspAccumulator *src = &accA;
    if (specialId >= 0x188 && specialId <= 0x18f) {
      src = &accB;
    }
    if (specialId >= 0x1d0 && specialId <= 0x1ef) {
      src = &accB;
    }

    int32_t value = saturated ? src->historySat24(PIPELINE_WRITE_DELAY)
                              : src->historyRaw24(PIPELINE_WRITE_DELAY);

    // Serial Output
    if (specialId >= 0x1b0 && specialId <= 0x1ef) {
      sioOutput[(specialId - 0x1b0) & 0x1f] = value;
    }

    // Branching
    else if (specialId >= 0x172 && specialId <= 0x177) {
      uint16_t jmpDest = coefs[pc];
      bool shouldJump = false;

      if (specialId == 0x172) { // JMP
        shouldJump = true;
      } else if (specialId == 0x173) { // JMP accA > 0
        shouldJump = value > 0;
      } else if (specialId == 0x174) { // JMP accA sat overflow
        shouldJump = accA.historyRawFull(PIPELINE_WRITE_DELAY) > 0x7fffff ||
                     accA.historyRawFull(PIPELINE_WRITE_DELAY) < -0x800000;
      } else if (specialId == 0x175) { // JMP accA < 0
        shouldJump = value < 0;
      } else if (specialId == 0x176) { // JMP accA == 0
        shouldJump = value == 0;
      } else if (specialId == 0x177) { // JMP accA >= 0
        shouldJump = value >= 0;
      }

      if (shouldJump) {
        pc = jmpDest - 1;
      }
    }

    // Host interface
    else if (specialId == 0x182 || specialId == 0x18a) {
      hostReadback = value;
    }

    // ERAM write latch
    else if (specialId == 0x183 || specialId == 0x18b) {
      eramWriteLatch = value;
    }

    // ERAM second tap pos
    else if (specialId == 0x185 || specialId == 0x18d) {
      eramVarOffset = (value & 0xffffff) >> 10;
      mulCoefA = (value & 0x3ff) << 13;
      mulCoefB = value;
    }

    // Mult coefficients
    else if (specialId == 0x186 || specialId == 0x18e) {
      mulCoefA = value;
    } else if (specialId == 0x187 || specialId == 0x18f) {
      mulCoefB = value;
    }
  }
};
