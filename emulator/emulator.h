#include <cstdint>
#include <fstream>
#include <limits>
#include <queue>
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
  int64_t acc = 0;
  std::array<int64_t, 8> hist{};
  std::size_t head = 0;

public:
  void set(int32_t v) { acc = v; }
  void add(int32_t v) { acc += v; }

  void abs() {
    if (acc < 0)
      acc = -acc;
  }

  int32_t sat24() const { return clamp_24(acc); }
  int32_t raw24() const { return sign_extend_24(acc & MASK); }

  int32_t historySat24(std::size_t n) const {
    std::size_t idx = (head - n) & 7;
    return clamp_24(hist[idx]);
  }
  int32_t historyRaw24(std::size_t n) const {
    std::size_t idx = (head - n) & 7;
    return sign_extend_24(hist[idx] & MASK);
  }

  void fwdPipeline() { head = (head + 1) & 7; }
  void storePipeline() { hist[head] = acc; }
};

struct RamOperationStage2 {
  bool isWrite = false;
  uint16_t addr = 0;
  int32_t writeData = 0;
  bool active = false;
  uint8_t stage = 0;

  int32_t *eram;
  std::queue<int32_t> *eramReadFifo;

  RamOperationStage2(int32_t *eram, std::queue<int32_t> *eramReadFifo)
      : eram(eram), eramReadFifo(eramReadFifo) {}

  void start(bool isWrite, uint16_t addr, uint8_t stage) {
    this->isWrite = isWrite;
    this->addr = addr;
    this->active = true;
    this->stage = stage;
  }

  void tick(int32_t eramWriteLatch) {
    writeData = eramWriteLatch;

    if (active) {
      if (stage == 11 && isWrite) {
        active = false;
        eram[addr] = writeData >> 4;
      }

      if (stage == 11 && !isWrite) {
        active = false;
        eramReadFifo->push(eram[addr] << 4);
      }

      stage += 1;
    }
  }
};

struct RamOperationStage1 {
  uint32_t addr = 0;
  uint8_t stage = 0;
  bool isWrite = false;
  bool active = false;

  void startTransaction(uint8_t command, uint16_t baseAddr,
                        int32_t offsetAddr) {
    if (active) {
      printf("stage 1 already active: %02x\n", command);
    }

    bool shouldUseOffset = (command & 0x06) == 0x04;
    addr = (baseAddr + (shouldUseOffset ? (offsetAddr >> 10) : 0)) & 0xffff;
    stage = 0;
    isWrite = (command & 0x06) == 0x06;
    active = true;
  }

  bool sendCommand(RamOperationStage2 &ramStage2, uint8_t command,
                   uint16_t baseAddr, int32_t offsetAddr) {
    if (active) {
      stage += 1;
      uint16_t incr = command << ((stage - 1) * 3);

      if (stage < 6) {
        addr = addr + incr;
      } else {
        if ((command & 1 && stage == 6)) {
          addr = addr + incr;
        }

        // pass to next stage
        ramStage2.start(isWrite, addr, stage);
        active = false;

        if (command > 0x1) {
          startTransaction(command, baseAddr, offsetAddr);
        }

        return true;
      }
    } else {
      if (command == 0x00) {
        // nop
      } else if (command > 0x01) {
        startTransaction(command, baseAddr, offsetAddr);
      } else {
        printf("ERAM invalid stage: %02x\n", command);
      }
    }

    return false;
  }
};

static constexpr int pipelineWriteDelay = 3;

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
  // TODO: this should be 16 bit, not 32
  int32_t eram[0x10000] = {0};
  uint16_t eramPos = 0;
  std::queue<int32_t> eramReadFifo;
  RamOperationStage1 ramStage1;
  RamOperationStage2 ramStage2;

  // Pipeline
  uint8_t prevRR = 0;

  // Special regs
  int32_t eramWriteLatch = 0;    // 0x10
  int32_t eramSecondTapOffs = 0; // 0x13
  int32_t multiplCoef1 = 0;      // 0x14
  int32_t multiplCoef2 = 0;      // 0x15
  int32_t audioOut = 0;          // 0x18
  int32_t audioIn = 0;           // 0x1e

  LspState() : ramStage2{RamOperationStage2(eram, &eramReadFifo)} {}

  void runProgram() {
    int total = 0;
    for (pc = 0x80; pc < 0x200 && total < 384; pc++, total++) {
      if (pc >= (0x80 + 384 / 2))
        audioIn = audioInL;
      else
        audioIn = audioInR;

      uint32_t instr = iram[pc];
      accA.fwdPipeline();
      accB.fwdPipeline();
      step(instr);
      accA.storePipeline();
      accB.storePipeline();

      if (pc >= (0x80 + 384 / 2))
        audioOutL = audioOut;
      else
        audioOutR = audioOut;
    }

    bufferPos = (bufferPos - 1) & 0x7f;
    eramPos -= 1;

    if (eramReadFifo.size() > 0) {
      printf("ERAM read FIFO overflow: %zu\n", eramReadFifo.size());
    }
  }

private:
  void step(uint32_t instr) {
    uint8_t ii = (instr >> 16) & 0xff;
    uint8_t rr = (instr >> 8) & 0xff;
    int8_t cc = (int8_t)(instr & 0xff);

    uint8_t opcode = ii & 0xe0;
    uint8_t extRamCtrl = ii & 0x7;

    if (opcode == 0x00 || opcode == 0x20 || opcode == 0x40 || opcode == 0x60 ||
        opcode == 0xa0) {
      doInstrMac(ii, rr, cc);
    } else if (opcode == 0x80) {
      doInstrMul(ii, rr, cc);
    } else if (opcode == 0xc0 || opcode == 0xe0) {
      doInstrSpecialReg(ii, rr, cc);
    }

    commonDoEram(extRamCtrl);

    prevRR = rr;
  }

  void commonDoEram(uint8_t command) {
    ramStage1.sendCommand(ramStage2, command, eramPos, eramSecondTapOffs);
    ramStage2.tick(eramWriteLatch);
  }

  void commonDoStore(uint8_t ii, uint8_t rr) {
    uint8_t memOffs = rr & 0x7f;
    uint8_t writeCtrl = ii & 0x18;
    if (writeCtrl == 0x00) {
      // nop
    } else if (writeCtrl == 0x08) {
      writeMemOffs(memOffs, accA.historySat24(pipelineWriteDelay));
    } else if (writeCtrl == 0x10) {
      writeMemOffs(memOffs, accB.historySat24(pipelineWriteDelay));
    } else if (writeCtrl == 0x18) {
      writeMemOffs(memOffs, accA.historyRaw24(pipelineWriteDelay));
    }
  }

  int32_t commonGetMemOrImmediate(uint8_t rr, int8_t cc) {
    uint8_t mulScaler = (rr & 0x80) != 0 ? 5 : 7;
    uint8_t memOffs = rr & 0x7f;
    int64_t incr = readMemOffs(memOffs) * cc;
    if (memOffs == 1)
      incr = cc << 7;
    if (memOffs == 2)
      incr = cc << 12;
    if (memOffs == 3)
      incr = cc << 17;
    if (memOffs == 4)
      incr = cc << 22;
    incr >>= mulScaler;
    return incr;
  }

  void doInstrMac(uint8_t ii, uint8_t rr, int8_t cc) {
    bool absValue = (ii & 0xe0) == 0xa0;
    bool replaceAcc = ((ii & 0x20) != 0) || absValue;
    DspAccumulator &acc = (ii & 0x40) != 0 ? accB : accA;

    commonDoStore(ii, rr);

    int32_t incr = commonGetMemOrImmediate(rr, cc);
    if (replaceAcc) {
      acc.set(incr);
    } else {
      acc.add(incr);
    }

    if (absValue) {
      acc.abs();
    }
  }

  void doInstrMul(uint8_t ii, uint8_t rr, int8_t cc) {
    uint8_t memOffs = rr & 0x7f;
    uint8_t mulScaler = (rr & 0x80) != 0 ? 5 : 7;
    bool lower = (cc & 0x40) != 0;
    DspAccumulator &acc = (cc & 0x10) != 0 ? accB : accA;
    bool negate = (cc & 0x4) != 0;
    bool replaceAcc = (cc & 0x8) != 0;
    int32_t opB = (cc & 0x2) != 0 ? multiplCoef2 : multiplCoef1;

    commonDoStore(ii, memOffs);

    if (lower) {
      opB &= 0xffff;
      opB >>= 9;
    } else {
      opB >>= 16;
    }

    int64_t opA = readMemOffs(memOffs);
    if (memOffs == 1)
      opA = cc << 7;
    if (memOffs == 2)
      opA = cc << 12;
    if (memOffs == 3)
      opA = cc << 17;
    if (memOffs == 4)
      opA = cc << 22;

    int64_t result = 0;
    if (cc == 0x00) {
      result = 0;
    } else if (cc == 0x04 || cc == 0x08 || cc == 0xc) {
      printf("UNIMPLEMENTED %02x %02x %02x\n", ii, rr, cc);
    } else {
      result = opA * opB;
    }

    result >>= mulScaler;
    if (lower) {
      result >>= 7;
    }

    if (negate) {
      result = -result;
    }
    if (!replaceAcc || lower) {
      result += acc.sat24();
    }
    acc.set(result);
  }

  void doInstrSpecialReg(uint8_t ii, uint8_t rr, int8_t cc) {
    DspAccumulator &accDest = (ii & 0x20) != 0 ? accB : accA;
    uint8_t writeCtrl = ii & 0x18;
    uint8_t mulScaler = (rr & 0x80) != 0 ? 5 : 7;
    bool useSpecial = (rr & 0x40) != 0;
    bool replaceAcc = (rr & 0x20) != 0;
    uint8_t specialSlot = rr & 0x1f;

    if (!useSpecial) {
      printf("UNIMPLEMENTED %02x %02x %02x\n", ii, rr, cc);
      return;
    }

    // special case 50/d0
    if (writeCtrl == 0x00) {
      if (specialSlot == 0x10) {
        uint8_t prevMem = prevRR & 0x7f;

        int64_t incr = readMemOffs(prevMem) * (uint8_t)cc;
        incr >>= 7;

        if (prevMem == 1 || prevMem == 2 || prevMem == 3 || prevMem == 4) {
          int prevShift = 0;
          if (prevMem == 2)
            prevShift = 5;
          else if (prevMem == 3)
            prevShift = 10;
          else if (prevMem == 4)
            prevShift = 15;
          incr = ((uint8_t)cc) << prevShift;
        }

        incr >>= 1;
        incr >>= mulScaler;

        if (replaceAcc) {
          accDest.set(incr);
        } else {
          accDest.add(incr);
        }
      } else {
        printf("UNIMPLEMENTED %02x %02x %02x\n", ii, rr, cc);
      }
      return;
    }

    int64_t src = 0;
    if (writeCtrl == 0x08) {
      src = accA.historySat24(pipelineWriteDelay);
    } else if (writeCtrl == 0x10) {
      src = accB.historySat24(pipelineWriteDelay);
    } else if (writeCtrl == 0x18) {
      src = accA.historyRaw24(pipelineWriteDelay);
    }

    bool updatesAcc = false;

    if (specialSlot == 0x0d) { // jmp if <0
      // TODO: probably wrong
      if (src < 0) {
        pc = (((uint8_t)cc) << 1) - 1;
      }
      updatesAcc = true; // TODO: check
    }

    else if (specialSlot == 0x0e) { // jmp if >=0
      if (src >= 0) {
        pc = (((uint8_t)cc) << 1) - 1;
      }
      updatesAcc = true; // TODO: check
    }

    else if (specialSlot == 0x0f) { // jmp
      pc = (((uint8_t)cc) << 1) - 1;
      updatesAcc = true; // TODO: check
    }

    else if (specialSlot == 0x10) { // eram write latch
      eramWriteLatch = src;
    }

    else if (specialSlot == 0x13) { // eram second tap offset
      eramSecondTapOffs = src;
    }

    else if (specialSlot == 0x14) { // multiplCoef1
      multiplCoef1 = src;
    }

    else if (specialSlot == 0x15) { // multiplCoef2
      multiplCoef2 = src;
    }

    else if (specialSlot == 0x18) { // audio out
      audioOut = src;
      writeMemOffs(0x78, src);
      updatesAcc = true;
    }

    else if (specialSlot == 0x1a) { // eram read 1
      if (!eramReadFifo.empty()) {
        src = eramReadFifo.front();
        eramReadFifo.pop();
      } else {
        printf("%04x: eram read 1 empty\n", pc);
      }
      writeMemOffs(0x7a, src);
      updatesAcc = true;
    }

    else if (specialSlot == 0x1b) { // eram read 2
      if (!eramReadFifo.empty()) {
        src = eramReadFifo.front();
        eramReadFifo.pop();
      } else {
        printf("%04x: eram read 2 empty\n", pc);
      }
      writeMemOffs(0x7b, src);
      updatesAcc = true;
    }

    else if (specialSlot == 0x1c) { // eram read 3
      if (!eramReadFifo.empty()) {
        src = eramReadFifo.front();
        eramReadFifo.pop();
      } else {
        printf("%04x: eram read 3 empty\n", pc);
      }
      writeMemOffs(0x7c, src);
      updatesAcc = true;
    }

    else if (specialSlot == 0x1d) { // eram read 4
      if (!eramReadFifo.empty()) {
        src = eramReadFifo.front();
        eramReadFifo.pop();
      } else {
        printf("%04x: eram read 4 empty\n", pc);
      }
      writeMemOffs(0x7d, src);
      updatesAcc = true;
    }

    else if (specialSlot == 0x1e) { // audio in
      src = audioIn;
      writeMemOffs(0x7e, src);
      updatesAcc = true;
    }

    else {
      printf("%04x: unknown special write %02x=%06x\n", pc, specialSlot,
             (int32_t)src);
      return;
    }

    if (updatesAcc) {
      src *= cc;
      src >>= mulScaler;
      if (replaceAcc) {
        accDest.set(src);
      } else {
        accDest.add(src);
      }
    }
  }

  void writeMemOffs(uint8_t memOffs, int32_t value) {
    uint32_t ramPos = ((uint32_t)memOffs + bufferPos) & 0x7f;
    iram[ramPos] = value;
  }

  int64_t readMemOffs(uint8_t memOffs) {
    uint32_t ramPos = ((uint32_t)memOffs + bufferPos) & 0x7f;
    return iram[ramPos];
  }
};
