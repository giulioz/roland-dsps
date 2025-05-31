#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "emulator.h"

void test1() {
  LspState state;
  state.audioInL = state.audioInR = sign_extend_24(0xffffc0);
  state.iram[350 + 0x80] = 0x00200142;
  state.iram[353 + 0x80] = 0x00080000;
  state.iram[358 + 0x80] = 0x00c8fe50;
  state.iram[362 + 0x80] = 0x00080100;
  state.iram[367 + 0x80] = 0x00000130;
  state.iram[368 + 0x80] = 0x00180500;
  state.iram[370 + 0x80] = 0x00180600;
  state.runProgram();
  for (size_t i = 0; i < 0x80; i++) {
    if (i == 0x00)
      assert(state.iram[i] == 0x000042);
    else if (i == 0x01)
      assert(state.iram[i] == sign_extend_24(0xffff60));
    else if (i == 0x05)
      assert(state.iram[i] == sign_extend_24(0xffff60));
    else if (i == 0x06)
      assert(state.iram[i] == sign_extend_24(0xffff90));
    else if (i == 0x7e)
      assert(state.iram[i] == sign_extend_24(0xffffc0));
    else
      assert(state.iram[i] == 0x000000);
  }
}

void test2_io() {
  LspState state;
  state.audioInL = state.audioInR = sign_extend_24(0xffffc0);
  state.iram[0 + 0x80] = 0x0020787f;
  state.iram[1 + 0x80] = 0x00c87e7f;
  state.iram[2 + 0x80] = 0x00000000;
  state.iram[3 + 0x80] = 0x0028f57f;
  state.iram[4 + 0x80] = 0x00287520;
  state.iram[5 + 0x80] = 0x00208120;
  state.iram[6 + 0x80] = 0x0008757f;
  state.iram[7 + 0x80] = 0x00087e00;
  state.iram[8 + 0x80] = 0x00000000;
  state.iram[9 + 0x80] = 0x00c85800;
  state.iram[17 + 0x80] = 0x00200101;
  state.iram[21 + 0x80] = 0x00080000;
  state.runProgram();
  for (size_t i = 0; i < 0x80; i++) {
    printf("%04x: %06x\n", i, state.iram[i] & 0xffffff);
  }
  for (size_t i = 0; i < 0x80; i++) {
    if (i == 0x00)
      assert(state.iram[i] == sign_extend_24(0x000001));
    else if (i == 0x78)
      assert(state.iram[i] == sign_extend_24(0x000080));
    else if (i == 0x7e)
      assert(state.iram[i] == sign_extend_24(0xfffff0));
    else
      assert(state.iram[i] == 0x000000);
  }
}

void test3_thru() {
  LspState state;
  state.audioInL = state.audioInR = sign_extend_24(0xffffc0);
  state.iram[0 + 0x80] = 0x0020787f;
  state.iram[1 + 0x80] = 0x00c87e7f;
  state.iram[2 + 0x80] = 0x00000000;
  state.iram[3 + 0x80] = 0x0028f57f;
  state.iram[4 + 0x80] = 0x00287520;
  state.iram[5 + 0x80] = 0x00208120;
  state.iram[6 + 0x80] = 0x0008757f;
  state.iram[7 + 0x80] = 0x00087e00;
  state.iram[8 + 0x80] = 0x00000000;
  state.iram[9 + 0x80] = 0x00c85800;
  state.iram[10 + 0x80] = 0x0020ff20;
  state.iram[11 + 0x80] = 0x0020fe20;
  state.iram[12 + 0x80] = 0x00000000;
  state.iram[13 + 0x80] = 0x00087900;
  state.iram[14 + 0x80] = 0x00087700;
  state.iram[370 + 0x80] = 0x0020797f;
  state.iram[371 + 0x80] = 0x00c87e7f;
  state.iram[372 + 0x80] = 0x00000000;
  state.iram[373 + 0x80] = 0x0028f57f;
  state.iram[374 + 0x80] = 0x00287520;
  state.iram[375 + 0x80] = 0x00208120;
  state.iram[376 + 0x80] = 0x0008757f;
  state.iram[377 + 0x80] = 0x00087e00;
  state.iram[378 + 0x80] = 0x00000000;
  state.iram[379 + 0x80] = 0x00c85800;
  state.iram[380 + 0x80] = 0x00c86ffe;
  state.runProgram();
  for (size_t i = 0; i < 0x80; i++) {
    printf("%04x: %06x\n", i, state.iram[i] & 0xffffff);
  }
  for (size_t i = 0; i < 0x80; i++) {
    if (i == 0x77)
      assert(state.iram[i] == sign_extend_24(0xfffff0));
    else if (i == 0x78)
      assert(state.iram[i] == sign_extend_24(0x000080));
    else if (i == 0x79)
      assert(state.iram[i] == sign_extend_24(0xfffff0));
    else if (i == 0x7e)
      assert(state.iram[i] == sign_extend_24(0xfffff0));
    else if (i == 0x7f)
      assert(state.iram[i] == sign_extend_24(0xfffff0));
    else
      assert(state.iram[i] == 0x000000);
  }
}

void test4_abs() {
  LspState state;
  state.iram[1 + 0x80] = 0x200101;
  state.iram[4 + 0x80] = 0x080000;
  state.iram[11 + 0x80] = 0x2003ff;
  state.iram[14 + 0x80] = 0x081500;
  state.iram[23 + 0x80] = 0x200142;
  state.iram[24 + 0x80] = 0x600143;
  state.iram[28 + 0x80] = 0xa0157f;
  state.iram[32 + 0x80] = 0x080400;
  state.iram[33 + 0x80] = 0x100500;
  state.iram[34 + 0x80] = 0x180600;
  state.runProgram();
  for (size_t i = 0; i < 0x80; i++) {
    if (i == 0x00)
      assert(state.iram[i] == sign_extend_24(0x000001));
    else if (i == 0x04)
      assert(state.iram[i] == sign_extend_24(0x0003f8));
    else if (i == 0x05)
      assert(state.iram[i] == sign_extend_24(0x000043));
    else if (i == 0x06)
      assert(state.iram[i] == sign_extend_24(0x0003f8));
    else if (i == 0x15)
      assert(state.iram[i] == sign_extend_24(0xfffc00));
    else
      assert(state.iram[i] == 0x000000);
  }
}

int main() {
  test1();
  // test2_io();
  // test3_thru();
  test4_abs();
  return 0;
}
