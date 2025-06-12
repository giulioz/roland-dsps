#pragma once

#include <cstdint>
#include <fstream>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <string>

static constexpr int32_t sign_extend_24(int32_t x) {
  x &= 0xffffff;
  if (x & 0x800000) // If sign bit is set
    x |= ~0xffffff;
  return x;
}

static constexpr int DATA_BITS = 24;
static constexpr int64_t MIN_VAL = -(1LL << (DATA_BITS - 1));
static constexpr int64_t MAX_VAL = (1LL << (DATA_BITS - 1)) - 1;
static constexpr int64_t MASK = (1LL << DATA_BITS) - 1;
static constexpr int32_t clamp_24(int64_t v) {
  if (v > MAX_VAL)
    return static_cast<int32_t>(MAX_VAL);
  if (v < MIN_VAL)
    return static_cast<int32_t>(MIN_VAL);
  return static_cast<int32_t>(v);
}

class DspAccumulator {
  int32_t acc = 0;
  int32_t hist[4] = {0};
  std::size_t head = 0;

public:
  inline void set(int32_t v) { acc = v; }
  inline void add(int32_t v) { acc += v; }

  inline void abs() {
    if (acc < 0)
      acc = -acc;
  }

  inline int32_t sat24() const { return clamp_24(acc); }
  inline int32_t raw24() const { return sign_extend_24(acc & MASK); }

  inline int32_t historySat24(std::size_t n) const {
    std::size_t idx = (head - n) & 3;
    return clamp_24(hist[idx]);
  }
  inline int32_t historyRaw24(std::size_t n) const {
    std::size_t idx = (head - n) & 3;
    return sign_extend_24(hist[idx] & MASK);
  }

  inline int32_t historyRaw32(std::size_t n) const {
    std::size_t idx = (head - n) & 3;
    return hist[idx];
  }

  inline void fwdPipeline() { head = (head + 1) & 3; }
  inline void storePipeline() { hist[head] = acc; }
};

struct LspInstr {
  bool skip = true;

  uint8_t ii = 0;
  uint8_t rr = 0;
  int8_t cc = 0;

  uint8_t opcode = 0;
  uint8_t extRamCtrl = 0;

  uint8_t writeCtrl = 0;
  uint8_t memOffs = 0;
  uint8_t mulScaler = 0;

  bool useImmediate = false;
  int32_t immediateValue = 0;

  // mac
  bool isMac = false;
  bool macAbsValue = false;
  bool macReplaceAcc = false;
  DspAccumulator *macAcc = nullptr;

  // mul
  bool isMul = false;
  bool mulLower = 0;
  DspAccumulator *mulAcc = nullptr;
  bool mulNegate = false;
  bool mulReplaceAcc = false;
  bool mulCoefSelect = false; // false: coef1, true: coef2

  // special
  bool isSpecial = false;
  DspAccumulator *specialAccDest = nullptr;
  bool useSpecial = false;
  bool specialReplaceAcc = false;
  uint8_t specialSlot = 0;
  bool specialCase50d0 = false;
  bool jmpOnPositive = false;
  bool jmpOnNegative = false;
  bool jmpAlways = false;
  bool jmp = false;
  uint16_t jmpDest = 0;
  bool isAudioIn = false;
  bool isAudioOut = false;

  // eram
  bool eramRead = false;
  bool eramWrite = false;
  bool eramUseSecondTap = false;
  uint32_t eramBaseAddr = 0;

  // skip
  LspInstr();

  LspInstr(uint8_t ii, uint8_t rr, int8_t cc, DspAccumulator *accA,
           DspAccumulator *accB);

  void setJmp(const LspInstr &prev);

  void setEram(const LspInstr instrSoFar[], int instrPos);
};

class LspState {
public:
  // Public audio inteface
  int32_t audioInL = 0;
  int32_t audioInR = 0;
  int32_t audioOutL = 0;
  int32_t audioOutR = 0;

  // Internal state
  DspAccumulator accA;
  DspAccumulator accB;
  uint8_t bufferPos = 0;
  int32_t iram[0x200] = {0};
  uint16_t pc = 0x80;

  // External RAM
  int32_t eram[0x10000] = {0};
  uint16_t eramPos = 0;
  int32_t eramReadValue;

  // Pipeline
  uint8_t prevRR = 0;
  bool shouldJump = false;

  // Special regs
  int32_t eramWriteLatch = 0;    // 0x10
  int32_t eramSecondTapOffs = 0; // 0x13
  int32_t multiplCoef1 = 0;      // 0x14
  int32_t multiplCoef2 = 0;      // 0x15
  int32_t audioOut = 0;          // 0x18
  int32_t audioIn = 0;           // 0x1e

  LspInstr instrCache[384];

  void runProgram();
  void optimiseProgram();

private:
  void step(const LspInstr &instr);
  void commonDoStore(const LspInstr &instr);
  void doInstrMac(const LspInstr &instr);
  void doInstrMul(const LspInstr &instr);
  void doInstrSpecialReg(const LspInstr &instr);
  void writeMemOffs(uint8_t memOffs, int32_t value);
  int64_t readMemOffs(uint8_t memOffs);
};
