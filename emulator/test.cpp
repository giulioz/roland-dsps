#include <stdio.h>
#include <stdlib.h>

#include "emulator.h"

void expectEqual(const char *where, int32_t actual, int32_t expected) {
  if (actual != expected) {
    fprintf(stderr, "FAIL %s: Expected %06x, got %06x\n", where,
            expected & 0xffffff, actual & 0xffffff);
  }
}

void testBasic() {
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
      expectEqual("testBasic", state.iram[i], 0x000042);
    else if (i == 0x01)
      expectEqual("testBasic", state.iram[i], sign_extend_24(0xffff60));
    else if (i == 0x05)
      expectEqual("testBasic", state.iram[i], sign_extend_24(0xffff60));
    else if (i == 0x06)
      expectEqual("testBasic", state.iram[i], sign_extend_24(0xffff90));
    else if (i == 0x7e)
      expectEqual("testBasic", state.iram[i], sign_extend_24(0xffffc0));
    else
      expectEqual("testBasic", state.iram[i], 0x000000);
  }
}

void testIo() {
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
    if (i == 0x00)
      expectEqual("testIo", state.iram[i], sign_extend_24(0x000001));
    else if (i == 0x78)
      expectEqual("testIo", state.iram[i], sign_extend_24(0x000080));
    else if (i == 0x7e)
      expectEqual("testIo", state.iram[i], sign_extend_24(0xfffff0));
    else
      expectEqual("testIo", state.iram[i], 0x000000);
  }
}

void testAbs() {
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
      expectEqual("testAbs", state.iram[i], sign_extend_24(0x000001));
    else if (i == 0x04)
      expectEqual("testAbs", state.iram[i], sign_extend_24(0x0003f8));
    else if (i == 0x05)
      expectEqual("testAbs", state.iram[i], sign_extend_24(0x000043));
    else if (i == 0x06)
      expectEqual("testAbs", state.iram[i], sign_extend_24(0x0003f8));
    else if (i == 0x15)
      expectEqual("testAbs", state.iram[i], sign_extend_24(0xfffc00));
    else
      expectEqual("testAbs", state.iram[i], 0x000000);
  }
}

void testMultSimple() {
  LspState state;
  state.iram[9 + 0x80] = 0x200242;
  state.iram[13 + 0x80] = 0x082000;
  state.iram[17 + 0x80] = 0x20043f;
  state.iram[21 + 0x80] = 0xc85400;
  state.iram[25 + 0x80] = 0x80a009;
  state.iram[31 + 0x80] = 0x080100;
  state.iram[32 + 0x80] = 0x180200;

  // 09: replace pos >> 5
  state.bufferPos = 0;
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x0007fe));

  // 09: replace pos >> 7
  state.iram[25 + 0x80] = 0x802009;
  state.bufferPos = 0;
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x0001ff));

  // 01: accumulate pos
  state.iram[25 + 0x80] = 0x802001;
  state.bufferPos = 0;
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x1f81ff));
  expectEqual("testMultSimple", state.iram[0x02], sign_extend_24(0x1f81ff));

  // 05: accumulate neg
  state.iram[25 + 0x80] = 0x802005;
  state.bufferPos = 0;
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x1f7e01));
  expectEqual("testMultSimple", state.iram[0x02], sign_extend_24(0x1f7e01));

  // 0a: accumulate other reg 0
  state.iram[25 + 0x80] = 0x80200a;
  state.bufferPos = 0;
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x000000));

  // 0a: accumulate other reg valid
  state.iram[21 + 0x80] = 0xc85500;
  state.iram[25 + 0x80] = 0x80200a;
  state.bufferPos = 0;
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x0001ff));
  state.iram[21 + 0x80] = 0xc85400;

  // overflow multiplier
  state.iram[9 + 0x80] = 0x200142;  // base
  state.iram[17 + 0x80] = 0x208442; // multiplier
  state.iram[25 + 0x80] = 0x802009; // mult command
  state.bufferPos = 0;
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x000041));
  expectEqual("testMultSimple", state.iram[0x02], sign_extend_24(0x000041));

  // overflow result
  state.iram[9 + 0x80] = 0x200442;  // base
  state.iram[17 + 0x80] = 0x208442; // multiplier
  state.iram[25 + 0x80] = 0x802009; // mult command
  state.bufferPos = 0;
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x20be00));
  expectEqual("testMultSimple", state.iram[0x02], sign_extend_24(0x20be00));

  // overflow value 1
  state.iram[9 + 0x80] = 0x208410;  // base
  state.iram[17 + 0x80] = 0x200442; // multiplier
  state.iram[25 + 0x80] = 0x802009; // mult command
  state.bufferPos = 0;
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x084000));
  expectEqual("testMultSimple", state.iram[0x02], sign_extend_24(0x084000));

  // overflow value 2
  state.iram[9 + 0x80] = 0x20847f;  // base
  state.iram[17 + 0x80] = 0x200442; // multiplier
  state.iram[25 + 0x80] = 0x802009; // mult command
  state.bufferPos = 0;
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x20ffff));
  expectEqual("testMultSimple", state.iram[0x02], sign_extend_24(0x20ffff));
}

void testMultStaged() {
  LspState state;
  state.iram[9 + 0x80] = 0x20037f;
  state.iram[13 + 0x80] = 0x082000;
  state.iram[17 + 0x80] = 0x20043f;
  state.iram[21 + 0x80] = 0xc85400;
  state.iram[25 + 0x80] = 0x80a009;
  state.iram[31 + 0x80] = 0x080100;
  state.iram[32 + 0x80] = 0x180200;
}

int main() {
  testBasic();
  testIo();
  testAbs();
  testMultSimple();
  testMultStaged();
  return 0;
}
