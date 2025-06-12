#include <stdio.h>
#include <stdlib.h>

#include "emulator.h"
#include "wavutils.h"

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
  state.optimiseProgram();
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
  state.optimiseProgram();
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
  {
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
    state.optimiseProgram();
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
  {
    LspState state;
    state.iram[15 + 0x80] = 0x2004a4;
    state.iram[23 + 0x80] = 0xb80580;
    state.iram[31 + 0x80] = 0x080100;
    state.iram[32 + 0x80] = 0x180200;
    state.optimiseProgram();
    state.runProgram();
    expectEqual("testAbs", state.iram[0x01], sign_extend_24(0x2e0000));
    expectEqual("testAbs", state.iram[0x02], sign_extend_24(0x2e0000));
    expectEqual("testAbs", state.iram[0x05], sign_extend_24(0xd20000));
  }
  {
    LspState state;
    state.iram[15 + 0x80] = 0x2001a4;
    state.iram[23 + 0x80] = 0xb80580;
    state.iram[31 + 0x80] = 0x080100;
    state.iram[32 + 0x80] = 0x180200;
    state.optimiseProgram();
    state.runProgram();
    expectEqual("testAbs", state.iram[0x01], sign_extend_24(0x00005c));
    expectEqual("testAbs", state.iram[0x02], sign_extend_24(0x00005c));
    expectEqual("testAbs", state.iram[0x05], sign_extend_24(0xffffa4));
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
  state.optimiseProgram();
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x0007fe));

  // 09: replace pos >> 7
  state.iram[25 + 0x80] = 0x802009;
  state.bufferPos = 0;
  state.optimiseProgram();
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x0001ff));

  // 01: accumulate pos
  state.iram[25 + 0x80] = 0x802001;
  state.bufferPos = 0;
  state.optimiseProgram();
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x1f81ff));
  expectEqual("testMultSimple", state.iram[0x02], sign_extend_24(0x1f81ff));

  // 05: accumulate neg
  state.iram[25 + 0x80] = 0x802005;
  state.bufferPos = 0;
  state.optimiseProgram();
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x1f7e01));
  expectEqual("testMultSimple", state.iram[0x02], sign_extend_24(0x1f7e01));

  // 0a: accumulate other reg 0
  state.iram[25 + 0x80] = 0x80200a;
  state.bufferPos = 0;
  state.optimiseProgram();
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x000000));

  // 0a: accumulate other reg valid
  state.iram[21 + 0x80] = 0xc85500;
  state.iram[25 + 0x80] = 0x80200a;
  state.bufferPos = 0;
  state.optimiseProgram();
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x0001ff));
  state.iram[21 + 0x80] = 0xc85400;

  // overflow multiplier
  state.iram[9 + 0x80] = 0x200142;  // base
  state.iram[17 + 0x80] = 0x208442; // multiplier
  state.iram[25 + 0x80] = 0x802009; // mult command
  state.bufferPos = 0;
  state.optimiseProgram();
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x000041));
  expectEqual("testMultSimple", state.iram[0x02], sign_extend_24(0x000041));

  // overflow result
  state.iram[9 + 0x80] = 0x200442;  // base
  state.iram[17 + 0x80] = 0x208442; // multiplier
  state.iram[25 + 0x80] = 0x802009; // mult command
  state.bufferPos = 0;
  state.optimiseProgram();
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x20be00));
  expectEqual("testMultSimple", state.iram[0x02], sign_extend_24(0x20be00));

  // overflow value 1
  state.iram[9 + 0x80] = 0x208410;  // base
  state.iram[17 + 0x80] = 0x200442; // multiplier
  state.iram[25 + 0x80] = 0x802009; // mult command
  state.bufferPos = 0;
  state.optimiseProgram();
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x084000));
  expectEqual("testMultSimple", state.iram[0x02], sign_extend_24(0x084000));

  // overflow value 2
  state.iram[9 + 0x80] = 0x20847f;  // base
  state.iram[17 + 0x80] = 0x200442; // multiplier
  state.iram[25 + 0x80] = 0x802009; // mult command
  state.bufferPos = 0;
  state.optimiseProgram();
  state.runProgram();
  expectEqual("testMultSimple", state.iram[0x01], sign_extend_24(0x20ffff));
  expectEqual("testMultSimple", state.iram[0x02], sign_extend_24(0x20ffff));
}

void testMultStaged() {
  {
    // 09-41
    LspState state;
    state.iram[9 + 0x80] = 0x20037f;
    state.iram[13 + 0x80] = 0x082000;
    state.iram[17 + 0x80] = 0x20043f;
    state.iram[21 + 0x80] = 0xc85400;
    state.iram[25 + 0x80] = 0x802009;
    state.iram[26 + 0x80] = 0x802041;
    state.iram[31 + 0x80] = 0x080100;
    state.iram[32 + 0x80] = 0x180200;
    state.optimiseProgram();
    state.runProgram();
    expectEqual("testMultStaged", state.iram[0x01], sign_extend_24(0x007d04));
    expectEqual("testMultStaged", state.iram[0x02], sign_extend_24(0x007d04));
  }

  {
    // 09-41
    LspState state;
    state.iram[9 + 0x80] = 0x20037f;
    state.iram[13 + 0x80] = 0x082000;
    state.iram[17 + 0x80] = 0x20043f;
    state.iram[18 + 0x80] = 0x000355;
    state.iram[22 + 0x80] = 0xc85400;
    state.iram[25 + 0x80] = 0x802009;
    state.iram[26 + 0x80] = 0x802041;
    state.iram[31 + 0x80] = 0x080100;
    state.iram[32 + 0x80] = 0x180200;
    state.optimiseProgram();
    state.runProgram();
    expectEqual("testMultStaged", state.iram[0x01], sign_extend_24(0x008249));
    expectEqual("testMultStaged", state.iram[0x02], sign_extend_24(0x008249));
  }

  {
    // 09-41
    LspState state;
    state.iram[9 + 0x80] = 0x20037f;
    state.iram[13 + 0x80] = 0x082000;
    state.iram[17 + 0x80] = 0x20043f;
    state.iram[18 + 0x80] = 0x000355;
    state.iram[22 + 0x80] = 0xc85400;
    state.iram[25 + 0x80] = 0x80a009;
    state.iram[26 + 0x80] = 0x80a041;
    state.iram[31 + 0x80] = 0x080100;
    state.iram[32 + 0x80] = 0x180200;
    state.optimiseProgram();
    state.runProgram();
    expectEqual("testMultStaged", state.iram[0x01], sign_extend_24(0x020925));
    expectEqual("testMultStaged", state.iram[0x02], sign_extend_24(0x020925));
  }

  {
    // 19-51
    LspState state;
    state.iram[9 + 0x80] = 0x20037f;
    state.iram[13 + 0x80] = 0x082000;
    state.iram[17 + 0x80] = 0x20043f;
    state.iram[18 + 0x80] = 0x000355;
    state.iram[22 + 0x80] = 0xc85400;
    state.iram[25 + 0x80] = 0x802019;
    state.iram[26 + 0x80] = 0x802051;
    state.iram[31 + 0x80] = 0x100100;
    state.iram[32 + 0x80] = 0x180200;
    state.optimiseProgram();
    state.runProgram();
    expectEqual("testMultStaged", state.iram[0x01], sign_extend_24(0x008249));
    expectEqual("testMultStaged", state.iram[0x02], sign_extend_24(0x20d400));
  }

  {
    // 0d-45
    LspState state;
    state.iram[9 + 0x80] = 0x20037f;
    state.iram[13 + 0x80] = 0x082000;
    state.iram[17 + 0x80] = 0x20043f;
    state.iram[18 + 0x80] = 0x000355;
    state.iram[22 + 0x80] = 0xc85400;
    state.iram[25 + 0x80] = 0x80200d;
    state.iram[26 + 0x80] = 0x802045;
    state.iram[31 + 0x80] = 0x080100;
    state.iram[32 + 0x80] = 0x180200;
    state.optimiseProgram();
    state.runProgram();
    expectEqual("testMultStaged", state.iram[0x01], sign_extend_24(0xff7db7));
    expectEqual("testMultStaged", state.iram[0x02], sign_extend_24(0xff7db7));
  }
}

void test50() {
  // With immediate
  {
    LspState state;
    state.iram[15 + 0x80] = 0x20022f;
    state.iram[16 + 0x80] = 0xc05060;
    state.iram[31 + 0x80] = 0x080100;
    state.optimiseProgram();
    state.runProgram();
    expectEqual("test50_imm", state.iram[0x01], sign_extend_24(0x0005ec));
  }
  {
    LspState state;
    state.iram[15 + 0x80] = 0x20022f;
    state.iram[16 + 0x80] = 0xc05090;
    state.iram[31 + 0x80] = 0x080100;
    state.optimiseProgram();
    state.runProgram();
    expectEqual("test50_imm", state.iram[0x01], sign_extend_24(0x0005f2));
  }
  {
    LspState state;
    state.iram[15 + 0x80] = 0x20032f;
    state.iram[16 + 0x80] = 0xc0d0f0;
    state.iram[31 + 0x80] = 0x080100;
    state.optimiseProgram();
    state.runProgram();
    expectEqual("test50_imm", state.iram[0x01], sign_extend_24(0x00cb00));
  }

  // With mem
  {
    LspState state;
    state.iram[9 + 0x80] = 0x200235;
    state.iram[13 + 0x80] = 0x083000;
    state.iram[24 + 0x80] = 0x20302f;
    state.iram[25 + 0x80] = 0xc0d0f0;
    state.iram[31 + 0x80] = 0x080100;
    state.optimiseProgram();
    state.runProgram();
    expectEqual("test50_mem", state.iram[0x01], sign_extend_24(0x00029f));
  }
  {
    LspState state;
    state.iram[9 + 0x80] = 0x200349;
    state.iram[13 + 0x80] = 0x083000;
    state.iram[24 + 0x80] = 0x203010;
    state.iram[25 + 0x80] = 0xc0504f;
    state.iram[31 + 0x80] = 0x080100;
    state.iram[32 + 0x80] = 0x180200;
    state.optimiseProgram();
    state.runProgram();
    expectEqual("test50_mem", state.iram[0x01], sign_extend_24(0x002534));
    expectEqual("test50_mem", state.iram[0x02], sign_extend_24(0x002534));
  }
  {
    LspState state;
    state.iram[9 + 0x80] = 0x200449;
    state.iram[13 + 0x80] = 0x083000;
    state.iram[24 + 0x80] = 0x203010;
    state.iram[25 + 0x80] = 0xc0d0ff;
    state.iram[31 + 0x80] = 0x080100;
    state.iram[32 + 0x80] = 0x180200;
    state.optimiseProgram();
    state.runProgram();
    expectEqual("test50_mem", state.iram[0x01], sign_extend_24(0x05b2dc));
    expectEqual("test50_mem", state.iram[0x02], sign_extend_24(0x05b2dc));
  }
}

void testRamWeird() {
  LspState state;
  state.iram[7 + 0x80] = 0x200100;
  state.iram[10 + 0x80] = 0xc85400;
  state.iram[14 + 0x80] = 0x200340;
  state.iram[15 + 0x80] = 0x060000;
  state.iram[23 + 0x80] = 0xc85000;
  state.iram[27 + 0x80] = 0x020000;
  state.iram[33 + 0x80] = 0x020000;
  state.iram[39 + 0x80] = 0xc87b00;
  state.iram[44 + 0x80] = 0x807b09;
  state.iram[45 + 0x80] = 0xc8da20;
  state.iram[46 + 0x80] = 0x807a05;
  state.iram[46 + 0x80] = 0x080100;
  for (size_t i = 0; i < 0x1ffff; i++) {
    state.optimiseProgram();
    state.runProgram();
  }
  expectEqual("testRamWeird", state.iram[0x01], sign_extend_24(0x010000));
}

void testEqualsWav(const char *programFilename, const char *inputFilename,
                   const char *expectFilename) {
  LspState state;

  // Load the program file
  FILE *pgmFile = fopen(programFilename, "r");
  if (!pgmFile) {
    fprintf(stderr, "Error opening program file: %s\n", programFilename);
    return;
  }
  char line[512] = {0};
  int iramIdx = 0x80;
  while (fgets(line, sizeof(line), pgmFile)) {
    int b1, b2, b3;
    const char *format = "%2x %2x %2x";
    if (line[4] == ':') {
      format = "%*[^:]: %2x %2x %2x";
    }
    if (sscanf(line, format, &b1, &b2, &b3) == 3) {
      int32_t value = (b1 << 16) | (b2 << 8) | b3;
      state.iram[iramIdx++] = value;
    }
  }
  fclose(pgmFile);

  std::vector<int16_t> audioSamples;
  int sampleRate = 0;
  int numChannels = 0;
  read_wav(inputFilename, audioSamples, sampleRate, numChannels);

  std::vector<int16_t> audioOutput;

  // Process audio samples
  for (size_t i = 0; i < audioSamples.size(); i += numChannels) {
    if (numChannels == 1) {
      state.audioInL = state.audioInR = audioSamples[i];
    } else if (numChannels == 2) {
      state.audioInL = audioSamples[i];
      state.audioInR = audioSamples[i + 1];
    }

    state.audioInL <<= 6;
    state.audioInR <<= 6;

    state.optimiseProgram();
    state.runProgram();

    state.audioOutL >>= 8;
    state.audioOutR >>= 8;

    audioOutput.push_back(static_cast<int16_t>(state.audioOutL));
    audioOutput.push_back(static_cast<int16_t>(state.audioOutR));
  }

  std::vector<int16_t> audioSamplesExpected;
  read_wav(expectFilename, audioSamplesExpected, sampleRate, numChannels);
  if (audioSamplesExpected.size() != audioOutput.size()) {
    fprintf(stderr, "%s: Output size mismatch: expected %zu, got %zu\n",
            programFilename, audioSamplesExpected.size(), audioOutput.size());
    return;
  }
  for (size_t i = 0; i < audioSamplesExpected.size(); i++) {
    if (audioSamplesExpected[i] != audioOutput[i]) {
      fprintf(stderr, "%s: Mismatch at sample %zu: expected %d, got %d\n",
              programFilename, i, audioSamplesExpected[i], audioOutput[i]);
      return;
    }
  }
}

int main() {
  testBasic();
  testIo();
  testAbs();
  testMultSimple();
  testMultStaged();
  test50();
  testRamWeird();

  testEqualsWav("reverb_perftest.txt", "input_guit.wav", "expected_reverb.wav");
  testEqualsWav("chorus_perftest.txt", "input_guit.wav", "expected_chorus.wav");
  testEqualsWav("auto3d_perftest.txt", "input_guit.wav", "expected_auto3d.wav");

  return 0;
}
