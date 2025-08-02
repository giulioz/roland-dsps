#pragma once

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

class Runtime {
public:
  int32_t audioInL = 0;
  int32_t audioInR = 0;
  int32_t audioOutL = 0;
  int32_t audioOutR = 0;

  // for testing
  int32_t iram[0x200] = {0};

  void runCompiled();

private:
  int32_t accA = 0;
  int32_t accB = 0;
  int32_t a_d1 = 0, a_d2 = 0, a_d3 = 0; // shift reg for accA
  int32_t b_d1 = 0, b_d2 = 0, b_d3 = 0; // shift reg for accB
  uint8_t bufferPos = 0;
  uint16_t eramPos = 0;
  int32_t eramWriteLatch = 0;
  int32_t eramSecondTapOffs = 0;
  int32_t eramReadValue = 0;
  int32_t multiplCoef1 = 0;
  int32_t multiplCoef2 = 0;
  bool shouldJump = false;
  int32_t eram[0x10000] = {0};

  inline int32_t readForStoreAccA() { return a_d3; }
  inline int32_t readForStoreAccB() { return b_d3; }
  inline void advanceAccA() {
    a_d3 = a_d2;
    a_d2 = a_d1;
    a_d1 = accA;
  }
  inline void advanceAccB() {
    b_d3 = b_d2;
    b_d2 = b_d1;
    b_d1 = accB;
  }

  inline void writeMemOffs(uint8_t memOffs, int32_t value) {
    uint32_t ramPos = ((uint32_t)memOffs + bufferPos) & 0x7f;
    iram[ramPos] = value;
  }
  inline int64_t readMemOffs(uint8_t memOffs) {
    uint32_t ramPos = ((uint32_t)memOffs + bufferPos) & 0x7f;
    return iram[ramPos];
  }
};
