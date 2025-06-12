#include "emulator.h"

static constexpr int pipelineWriteDelay = 3;

// skip
LspInstr::LspInstr() {}

LspInstr::LspInstr(uint8_t ii, uint8_t rr, int8_t cc, DspAccumulator *accA,
                   DspAccumulator *accB)
    : ii(ii), rr(rr), cc(cc) {
  skip = false;

  opcode = ii & 0xe0;
  extRamCtrl = ii & 0x7;

  writeCtrl = ii & 0x18;
  memOffs = rr & 0x7f;
  mulScaler = (rr & 0x80) != 0 ? 5 : 7;

  switch (memOffs) {
  case 1:
    useImmediate = true;
    immediateValue = (cc << 7) >> mulScaler;
    break;
  case 2:
    useImmediate = true;
    immediateValue = (cc << 12) >> mulScaler;
    break;
  case 3:
    useImmediate = true;
    immediateValue = (cc << 17) >> mulScaler;
    break;
  case 4:
    useImmediate = true;
    immediateValue = (cc << 22) >> mulScaler;
    break;
  }

  // mac
  isMac = opcode == 0x00 || opcode == 0x20 || opcode == 0x40 ||
          opcode == 0x60 || opcode == 0xa0;
  macAbsValue = opcode == 0xa0;
  macReplaceAcc = ((ii & 0x20) != 0) || macAbsValue;
  macAcc = (ii & 0x40) != 0 ? accB : accA;

  // mul
  isMul = opcode == 0x80;
  mulLower = (cc & 0x40) != 0;
  mulAcc = (cc & 0x10) != 0 ? accB : accA;
  mulNegate = (cc & 0x4) != 0;
  mulReplaceAcc = (cc & 0x8) != 0 && !mulLower;
  mulCoefSelect = (cc & 0x2) != 0;
  if (isMul) {
    switch (memOffs) {
    case 1:
      useImmediate = true;
      immediateValue = (cc << 7);
      break;
    case 2:
      useImmediate = true;
      immediateValue = (cc << 12);
      break;
    case 3:
      useImmediate = true;
      immediateValue = (cc << 17);
      break;
    case 4:
      useImmediate = true;
      immediateValue = (cc << 22);
      break;
    }
  }

  // special
  isSpecial = opcode == 0xc0 || opcode == 0xe0;
  specialAccDest = (ii & 0x20) != 0 ? accB : accA;
  useSpecial = (rr & 0x40) != 0;
  specialReplaceAcc = (rr & 0x20) != 0;
  specialSlot = rr & 0x1f;
  specialCase50d0 = writeCtrl == 0x00 && specialSlot == 0x10;
  isAudioOut = isSpecial && specialSlot == 0x18;
  isAudioIn = isSpecial && specialSlot == 0x1e;
}

void LspInstr::setJmp(const LspInstr &prev) {
  jmpOnPositive = prev.isSpecial && prev.specialSlot == 0x0e;
  jmpOnNegative = prev.isSpecial && prev.specialSlot == 0x0d;
  jmpAlways = prev.isSpecial && prev.specialSlot == 0x0f;
  jmp = jmpOnPositive || jmpOnNegative || jmpAlways;
  jmpDest = (((uint8_t)prev.cc) << 1) - 1 - 0x80;
}

void LspInstr::setEram(const LspInstr instrSoFar[], int instrPos) {
  eramRead =
      isSpecial && writeCtrl != 0 && specialSlot >= 0x1a && specialSlot <= 0x1d;
  eramWrite = isSpecial && writeCtrl != 0 && specialSlot == 0x10;

  int startInstrI = 0;
  if (eramWrite) {
    startInstrI = instrPos - 8;
  } else if (eramRead) {
    startInstrI = instrPos - 12;
  }

  if (!eramWrite && !eramRead)
    return;

  const LspInstr *curr = &instrSoFar[startInstrI];
  eramUseSecondTap = (curr->extRamCtrl & 0x06) == 0x04;
  eramBaseAddr = 0;
  curr++;
  for (int i = 1; i <= 6; i++) {
    uint16_t incr = curr->extRamCtrl << ((i - 1) * 3);
    if (eramUseSecondTap) {
      incr = 0x00;
      if (curr->extRamCtrl == 0x02 && i == 1) {
        incr = 0x01;
      } else if (curr->extRamCtrl != 0x00 && i != 6) {
        printf("ERAM invalid command: %02x\n", curr->extRamCtrl);
      }
    }

    if (i < 6) {
      eramBaseAddr += incr;
    } else if ((curr->extRamCtrl & 1 && i == 6)) {
      eramBaseAddr = eramBaseAddr + incr;
    }
    curr++;
  }
}

void LspState::optimiseProgram() {
  for (size_t pc = 0; pc < 384; pc++) {
    uint32_t instr = iram[0x80 + pc];
    if (instr == 0x000000) {
      instrCache[pc] = LspInstr();
    } else {
      uint8_t ii = (instr >> 16) & 0xff;
      uint8_t rr = (instr >> 8) & 0xff;
      int8_t cc = (int8_t)(instr & 0xff);
      instrCache[pc] = LspInstr(ii, rr, cc, &accA, &accB);
    }

    if (pc > 0) {
      instrCache[pc].setJmp(instrCache[pc - 1]);
    }

    instrCache[pc].setEram(instrCache, pc);
  }
}

void LspState::runProgram() {
  int total = 0;
  audioIn = audioInR;
  for (pc = 0; total < 384; pc++, total++) {
    const LspInstr instr = instrCache[pc];

    if (instr.isAudioIn && pc >= (384 / 2))
      audioIn = audioInL;

    accA.fwdPipeline();
    accB.fwdPipeline();
    step(instr);
    accA.storePipeline();
    accB.storePipeline();

    if (instr.jmp && shouldJump) {
      pc = instr.jmpDest;
      shouldJump = false;
    }

    if (instr.isAudioOut && pc < (384 / 2))
      audioOutR = audioOut;
  }
  audioOutL = audioOut;

  bufferPos = (bufferPos - 1) & 0x7f;
  eramPos -= 1;
}

void LspState::step(const LspInstr &instr) {
  if (instr.eramRead) {
    uint32_t addr = (eramPos + (int32_t)instr.eramBaseAddr +
                     (instr.eramUseSecondTap ? eramSecondTapOffs : 0)) &
                    0xffff;
    eramReadValue = eram[addr] << 4;
  }

  if (!instr.skip) {
    if (instr.isMac) {
      doInstrMac(instr);
    } else if (instr.isMul) {
      doInstrMul(instr);
    } else if (instr.isSpecial) {
      doInstrSpecialReg(instr);
    }
  }

  if (instr.eramWrite) {
    uint32_t addr = (eramPos + (int32_t)instr.eramBaseAddr) & 0xffff;
    eram[addr] = eramWriteLatch >> 4;
  }

  prevRR = instr.rr;
}

void LspState::commonDoStore(const LspInstr &instr) {
  switch (instr.writeCtrl) {
  case 0x08:
    writeMemOffs(instr.memOffs, accA.historySat24(pipelineWriteDelay));
    return;
  case 0x10:
    writeMemOffs(instr.memOffs, accB.historySat24(pipelineWriteDelay));
    return;
  case 0x18:
    writeMemOffs(instr.memOffs, accA.historyRaw24(pipelineWriteDelay));
    return;
  default:
    return;
  }
}

void LspState::doInstrMac(const LspInstr &instr) {
  commonDoStore(instr);

  int32_t incr = 0;
  if (instr.useImmediate) {
    incr = instr.immediateValue;
  } else {
    incr = (readMemOffs(instr.memOffs) * instr.cc) >> instr.mulScaler;
  }

  if (instr.macReplaceAcc) {
    instr.macAcc->set(incr);
  } else {
    instr.macAcc->add(incr);
  }

  if (instr.macAbsValue) {
    instr.macAcc->abs();
  }
}

void LspState::doInstrMul(const LspInstr &instr) {
  int32_t opB = instr.mulCoefSelect ? multiplCoef2 : multiplCoef1;

  commonDoStore(instr);

  if (instr.mulLower) {
    opB &= 0xffff;
    opB >>= 9;
  } else {
    opB >>= 16;
  }

  int64_t opA = 0;
  if (instr.useImmediate)
    opA = instr.immediateValue;
  else
    opA = readMemOffs(instr.memOffs);

  int64_t result = 0;
  if (instr.cc == 0x00) {
    result = 0;
  } else {
    result = opA * opB;
  }

  result >>= instr.mulScaler;
  if (instr.mulLower) {
    result >>= 7;
  }

  if (instr.mulNegate) {
    result = -result;
  }
  if (!instr.mulReplaceAcc) {
    result += instr.mulAcc->sat24();
  }
  instr.mulAcc->set(result);
}

void LspState::doInstrSpecialReg(const LspInstr &instr) {
  // special case 50/d0
  if (instr.specialCase50d0) {
    uint8_t prevMem = prevRR & 0x7f;

    int64_t incr = readMemOffs(prevMem) * (uint8_t)instr.cc;
    incr >>= 7;

    if (prevMem == 1 || prevMem == 2 || prevMem == 3 || prevMem == 4) {
      int prevShift = 0;
      if (prevMem == 2)
        prevShift = 5;
      else if (prevMem == 3)
        prevShift = 10;
      else if (prevMem == 4)
        prevShift = 15;
      incr = ((uint8_t)instr.cc) << prevShift;
    }

    incr >>= 1;
    incr >>= instr.mulScaler;

    if (instr.specialReplaceAcc) {
      instr.specialAccDest->set(incr);
    } else {
      instr.specialAccDest->add(incr);
    }
    return;
  }

  int64_t src = 0;
  if (instr.writeCtrl == 0x08) {
    src = accA.historySat24(pipelineWriteDelay);
  } else if (instr.writeCtrl == 0x10) {
    src = accB.historySat24(pipelineWriteDelay);
  } else if (instr.writeCtrl == 0x18) {
    src = accA.historyRaw24(pipelineWriteDelay);
  }

  bool updatesAcc = false;

  if (instr.specialSlot == 0x0d) { // jmp if <0
    shouldJump = src < 0;
    updatesAcc = true; // TODO: check
  }

  else if (instr.specialSlot == 0x0e) { // jmp if >=0
    shouldJump = src >= 0;
    updatesAcc = true; // TODO: check
  }

  else if (instr.specialSlot == 0x0f) { // jmp
    shouldJump = true;
    updatesAcc = true; // TODO: check
  }

  else if (instr.specialSlot == 0x10) { // eram write latch
    eramWriteLatch = src;
  }

  else if (instr.specialSlot == 0x13 &&
           instr.writeCtrl == 0x18) { // eram second tap offset
    int32_t val = accA.historyRaw32(pipelineWriteDelay);
    eramSecondTapOffs = val >> 10;
    multiplCoef1 = (val & 0x3ff) << 13;
  }

  else if (instr.specialSlot == 0x14) { // multiplCoef1
    multiplCoef1 = src;
  }

  else if (instr.specialSlot == 0x15) { // multiplCoef2
    multiplCoef2 = src;
  }

  else if (instr.specialSlot == 0x18) { // audio out
    audioOut = src;
    writeMemOffs(0x78, src);
    updatesAcc = true;
  }

  else if (instr.specialSlot == 0x1a) { // eram read 1
    src = eramReadValue;
    writeMemOffs(0x7a, src);
    updatesAcc = true;
  }

  else if (instr.specialSlot == 0x1b) { // eram read 2
    src = eramReadValue;
    writeMemOffs(0x7b, src);
    updatesAcc = true;
  }

  else if (instr.specialSlot == 0x1c) { // eram read 3
    src = eramReadValue;
    writeMemOffs(0x7c, src);
    updatesAcc = true;
  }

  else if (instr.specialSlot == 0x1d) { // eram read 4
    src = eramReadValue;
    writeMemOffs(0x7d, src);
    updatesAcc = true;
  }

  else if (instr.specialSlot == 0x1e) { // audio in
    src = audioIn;
    writeMemOffs(0x7e, src);
    updatesAcc = true;
  }

  else {
    printf("%04x: unknown special write %02x=%06x\n", pc, instr.specialSlot,
           (int32_t)src);
    return;
  }

  if (updatesAcc) {
    src *= instr.cc;
    src >>= instr.mulScaler;
    if (instr.specialReplaceAcc) {
      instr.specialAccDest->set(src);
    } else {
      instr.specialAccDest->add(src);
    }
  }
}

void LspState::writeMemOffs(uint8_t memOffs, int32_t value) {
  uint32_t ramPos = ((uint32_t)memOffs + bufferPos) & 0x7f;
  iram[ramPos] = value;
}

int64_t LspState::readMemOffs(uint8_t memOffs) {
  uint32_t ramPos = ((uint32_t)memOffs + bufferPos) & 0x7f;
  return iram[ramPos];
}
