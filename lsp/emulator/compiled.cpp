#include "runtime.h"
void Runtime::runCompiled() {
  int32_t accA_0;
  int32_t accA_1;
  int32_t accA_3;
  int32_t accA_4;
  int32_t accA_6;
  int32_t accB_6;
  int32_t accA_11;
  int32_t accA_16;
  int32_t accA_21;
  int32_t accA_28;
  int32_t accA_31;
  int32_t accA_34;
  int32_t accB_34;
  int32_t accA_53;
  int32_t accB_53;
  int32_t accA_54;
  int32_t accA_66;
  int32_t accB_66;
  int32_t accA_67;
  int32_t accA_79;
  int32_t accB_79;
  int32_t accA_80;
  int32_t accA_92;
  int32_t accB_92;
  int32_t accA_93;
  int32_t accA_97;
  int32_t accB_114;
  int32_t accA_131;
  int32_t accB_131;
  int32_t accA_133;
  int32_t accA_137;
  int32_t accB_137;
  int32_t accA_156;
  int32_t accB_156;
  int32_t accA_157;
  int32_t accA_162;
  int32_t accB_162;
  int32_t accB_180;
  int32_t accA_197;
  int32_t accB_197;
  int32_t accA_199;
  int32_t accA_203;
  int32_t accB_203;
  int32_t accA_222;
  int32_t accB_222;
  int32_t accA_223;
  int32_t accA_228;
  int32_t accB_228;
  int32_t accA_262;
  int32_t accA_265;
  int32_t accA_268;
  int32_t accA_302;
  int32_t accA_305;
  int32_t accA_308;
  int32_t accA_317;
  int32_t accA_323;
  int32_t accA_324;
  int32_t accA_333;
  int32_t accA_339;
  int32_t accA_340;
  int32_t accA_370;
  int32_t accA_371;
  int32_t accA_373;
  int32_t accA_374;
  int32_t accA_376;
  int32_t accB_376;

  // 080: 20 78 7f
  accA = (readMemOffs(120) * 127) >> 7;
  accA_0 = accA;

  // 081: c8 7e 7f
  writeMemOffs(0x7e, audioInR);
  accA = (audioInR * 127) >> 7;
  accA_1 = accA;

  // 082: 00 00 00

  // 083: 28 f5 20
  writeMemOffs(117, clamp_24(accA_0));
  accA = readMemOffs(117);
  accA_3 = accA;

  // 084: 28 75 7f
  writeMemOffs(117, clamp_24(accA_1));
  accA = (readMemOffs(117) * 127) >> 7;
  accA_4 = accA;

  // 085: 20 81 20
  accA = 128;
  // 086: 08 75 7f
  writeMemOffs(117, clamp_24(accA_3));
  accA += (readMemOffs(117) * 127) >> 7;
  accA_6 = accA;
  accB_6 = accB;

  // 087: 08 7e 00
  writeMemOffs(126, clamp_24(accA_4));

  // 088: 00 00 00

  // 089: c8 58 00
  audioOutR = clamp_24(accA_6);
  writeMemOffs(0x78, audioOutR);
  accA += (clamp_24(accA_6) * 0) >> 7;
  // 08a: 20 7f 40
  accA = (readMemOffs(127) * 64) >> 7;
  // 08b: 00 7e 40
  accA += (readMemOffs(126) * 64) >> 7;
  accA_11 = accA;

  // 08c: 00 00 00

  // 08d: 00 00 00

  // 08e: 28 75 7f
  writeMemOffs(117, clamp_24(accA_11));
  accA = (readMemOffs(117) * 127) >> 7;
  // 08f: c0 50 c0
  accA += ((readMemOffs(117) * 192) >> 7) >> 8;
  // 090: 00 88 20
  accA += readMemOffs(8);
  accA_16 = accA;

  // 091: 00 00 00

  // 092: 00 00 00

  // 093: 28 75 ff
  writeMemOffs(117, clamp_24(accA_16));
  accA = (readMemOffs(117) * -1) >> 7;
  // 094: c0 50 80
  accA += ((readMemOffs(117) * 128) >> 7) >> 8;
  // 095: 00 88 20
  accA += readMemOffs(8);
  accA_21 = accA;

  // 096: 00 00 00

  // 097: 00 00 00

  // 098: 08 07 00
  writeMemOffs(7, clamp_24(accA_21));

  // 099: 20 75 7f
  accA = (readMemOffs(117) * 127) >> 7;
  // 09a: c0 50 fc
  accA += ((readMemOffs(117) * 252) >> 7) >> 8;
  // 09b: 00 0a 00

  // 09c: c0 50 00
  accA += ((readMemOffs(10) * 0) >> 7) >> 8;
  accA_28 = accA;

  // 09d: 06 00 00

  // 09e: 00 00 00

  // 09f: 28 09 20
  writeMemOffs(9, clamp_24(accA_28));
  accA = (readMemOffs(9) * 32) >> 7;
  accA_31 = accA;

  // 0a0: 00 00 00

  // 0a1: 00 00 00

  // 0a2: 2e 75 7f
  writeMemOffs(117, clamp_24(accA_31));
  accA = (readMemOffs(117) * 127) >> 7;
  accA_34 = accA;
  accB_34 = accB;

  // 0a3: 02 00 00

  // 0a4: 05 00 00

  // 0a5: ca 50 00
  eramWriteLatch = clamp_24(accA_34);
  eram[(eramPos + 24576) & 0xffff] = eramWriteLatch >> 4;
  // 0a6: 06 00 00

  // 0a7: 01 00 00

  // 0a8: 00 00 00

  // 0a9: 02 00 00

  // 0aa: 00 00 00

  // 0ab: 00 00 00

  // 0ac: 05 00 00

  // 0ad: 04 00 00

  // 0ae: 06 00 00

  // 0af: c8 7a c0
  eramReadValue = eram[(eramPos + 917) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA = (eramReadValue * -64) >> 7;
  // 0b0: 06 00 00

  // 0b1: 00 00 00

  // 0b2: 00 00 00

  // 0b3: 00 00 00

  // 0b4: 00 00 00

  // 0b5: c8 5b 7f
  eramReadValue = eram[(eramPos + 26944) & 0xffff] << 4;
  writeMemOffs(123, eramReadValue);
  accA += (eramReadValue * 127) >> 7;
  accA_53 = accA;
  accB_53 = accB;

  // 0b6: 02 00 00
  accA_54 = accA;

  // 0b7: 07 00 00

  // 0b8: c9 50 00
  eramWriteLatch = clamp_24(accA_53);
  eram[(eramPos + 0) & 0xffff] = eramWriteLatch >> 4;
  // 0b9: 2c 75 40
  writeMemOffs(117, clamp_24(accA_54));
  accA = (readMemOffs(117) * 64) >> 7;
  // 0ba: 02 fa 20
  accA += readMemOffs(122);
  // 0bb: 00 00 00

  // 0bc: 00 00 00

  // 0bd: 06 00 00

  // 0be: 06 00 00

  // 0bf: 02 00 00

  // 0c0: 06 00 00

  // 0c1: 01 00 00

  // 0c2: c8 5a c0
  eramReadValue = eram[(eramPos + 1295) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA += (eramReadValue * -64) >> 7;
  accA_66 = accA;
  accB_66 = accB;

  // 0c3: 02 00 00
  accA_67 = accA;

  // 0c4: 06 00 00

  // 0c5: cf 50 00
  eramWriteLatch = clamp_24(accA_66);
  eram[(eramPos + 918) & 0xffff] = eramWriteLatch >> 4;
  // 0c6: 2d 75 40
  writeMemOffs(117, clamp_24(accA_67));
  accA = (readMemOffs(117) * 64) >> 7;
  // 0c7: 02 fa 20
  accA += readMemOffs(122);
  // 0c8: 00 00 00

  // 0c9: 00 00 00

  // 0ca: 06 00 00

  // 0cb: 00 00 00

  // 0cc: 02 00 00

  // 0cd: 04 00 00

  // 0ce: 02 00 00

  // 0cf: c8 5a c0
  eramReadValue = eram[(eramPos + 1406) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA += (eramReadValue * -64) >> 7;
  accA_79 = accA;
  accB_79 = accB;

  // 0d0: 02 00 00
  accA_80 = accA;

  // 0d1: 03 00 00

  // 0d2: c9 50 00
  eramWriteLatch = clamp_24(accA_79);
  eram[(eramPos + 1296) & 0xffff] = eramWriteLatch >> 4;
  // 0d3: 2f 75 40
  writeMemOffs(117, clamp_24(accA_80));
  accA = (readMemOffs(117) * 64) >> 7;
  // 0d4: 02 fa 20
  accA += readMemOffs(122);
  // 0d5: 00 00 00

  // 0d6: 00 00 00

  // 0d7: 06 00 00

  // 0d8: 07 00 00

  // 0d9: 07 00 00

  // 0da: 05 00 00

  // 0db: 02 00 00

  // 0dc: c8 5a 00
  eramReadValue = eram[(eramPos + 1483) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA += (eramReadValue * 0) >> 7;
  accA_92 = accA;
  accB_92 = accB;

  // 0dd: 00 00 00
  accA_93 = accA;

  // 0de: 00 00 00

  // 0df: c8 50 00
  eramWriteLatch = clamp_24(accA_92);
  eram[(eramPos + 1407) & 0xffff] = eramWriteLatch >> 4;
  // 0e0: 28 75 00
  writeMemOffs(117, clamp_24(accA_93));
  accA = (readMemOffs(117) * 0) >> 7;
  // 0e1: 00 fa 20
  accA += readMemOffs(122);
  accA_97 = accA;

  // 0e2: 00 00 00

  // 0e3: 00 00 00

  // 0e4: 2a a0 20
  writeMemOffs(32, clamp_24(accA_97));
  accA = readMemOffs(32);
  // 0e5: 01 00 00

  // 0e6: 06 00 00

  // 0e7: 00 00 00

  // 0e8: 06 00 00

  // 0e9: 01 00 00

  // 0ea: 00 00 00

  // 0eb: 00 00 00

  // 0ec: 00 00 00

  // 0ed: 00 00 00

  // 0ee: 60 11 2a
  accB = (readMemOffs(17) * 42) >> 7;
  // 0ef: e0 50 7b
  accB += ((readMemOffs(17) * 123) >> 7) >> 8;
  // 0f0: e8 5a 00
  eramReadValue = eram[(eramPos + 7217) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accB += (eramReadValue * 0) >> 7;
  // 0f1: 40 7a 55
  accB += (readMemOffs(122) * 85) >> 7;
  // 0f2: e0 50 83
  accB += ((readMemOffs(122) * 131) >> 7) >> 8;
  accB_114 = accB;

  // 0f3: 00 00 00

  // 0f4: 00 00 00

  // 0f5: 10 10 59
  writeMemOffs(16, clamp_24(accB_114));
  accA += (readMemOffs(16) * 89) >> 7;
  // 0f6: c0 50 c3
  accA += ((readMemOffs(16) * 195) >> 7) >> 8;
  // 0f7: 02 00 00

  // 0f8: 05 00 00

  // 0f9: 04 00 00

  // 0fa: 01 00 00

  // 0fb: 04 00 00

  // 0fc: 00 00 00

  // 0fd: 00 00 00

  // 0fe: 06 00 00

  // 0ff: 04 00 00

  // 100: 01 00 00

  // 101: 07 00 00

  // 102: 02 00 00

  // 103: c8 5a c0
  eramReadValue = eram[(eramPos + 2149) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA += (eramReadValue * -64) >> 7;
  accA_131 = accA;
  accB_131 = accB;

  // 104: 06 00 00

  // 105: 06 00 00
  accA_133 = accA;

  // 106: cc 50 00
  eramWriteLatch = clamp_24(accA_131);
  eram[(eramPos + 1484) & 0xffff] = eramWriteLatch >> 4;
  // 107: 01 00 00

  // 108: 2c 75 40
  writeMemOffs(117, clamp_24(accA_133));
  accA = (readMemOffs(117) * 64) >> 7;
  // 109: 00 fa 20
  accA += readMemOffs(122);
  accA_137 = accA;
  accB_137 = accB;

  // 10a: 02 00 00

  // 10b: 07 00 00

  // 10c: cb 50 00
  eramWriteLatch = clamp_24(accA_137);
  eram[(eramPos + 2150) & 0xffff] = eramWriteLatch >> 4;
  // 10d: 07 00 00

  // 10e: 07 00 00

  // 10f: 00 00 00

  // 110: 02 00 00

  // 111: 05 00 00

  // 112: 01 00 00

  // 113: 04 00 00

  // 114: 01 00 00

  // 115: 01 00 00

  // 116: c8 fa 20
  eramReadValue = eram[(eramPos + 4063) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA = (eramReadValue * 32) >> 5;
  // 117: 06 00 00

  // 118: 00 00 00

  // 119: 04 00 00

  // 11a: 07 00 00

  // 11b: 07 00 00

  // 11c: c8 5a c0
  eramReadValue = eram[(eramPos + 4877) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA += (eramReadValue * -64) >> 7;
  accA_156 = accA;
  accB_156 = accB;

  // 11d: 06 00 00
  accA_157 = accA;

  // 11e: 06 00 00

  // 11f: c9 50 00
  eramWriteLatch = clamp_24(accA_156);
  eram[(eramPos + 4064) & 0xffff] = eramWriteLatch >> 4;
  // 120: 2c 75 40
  writeMemOffs(117, clamp_24(accA_157));
  accA = (readMemOffs(117) * 64) >> 7;
  // 121: 01 fa 20
  accA += readMemOffs(122);
  // 122: 01 00 00
  accA_162 = accA;
  accB_162 = accB;

  // 123: 00 00 00

  // 124: 00 00 00

  // 125: c8 50 00
  eramWriteLatch = clamp_24(accA_162);
  eram[(eramPos + 4878) & 0xffff] = eramWriteLatch >> 4;
  // 126: 22 a0 20
  accA = readMemOffs(32);
  // 127: 00 00 00

  // 128: 01 00 00

  // 129: 05 00 00

  // 12a: 01 00 00

  // 12b: 03 00 00

  // 12c: 00 00 00

  // 12d: 00 00 00

  // 12e: 00 00 00

  // 12f: 00 00 00

  // 130: 60 13 2a
  accB = (readMemOffs(19) * 42) >> 7;
  // 131: e0 50 7b
  accB += ((readMemOffs(19) * 123) >> 7) >> 8;
  // 132: e8 5a 00
  eramReadValue = eram[(eramPos + 13128) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accB += (eramReadValue * 0) >> 7;
  // 133: 40 7a 55
  accB += (readMemOffs(122) * 85) >> 7;
  // 134: e0 50 83
  accB += ((readMemOffs(122) * 131) >> 7) >> 8;
  accB_180 = accB;

  // 135: 00 00 00

  // 136: 00 00 00

  // 137: 10 12 59
  writeMemOffs(18, clamp_24(accB_180));
  accA += (readMemOffs(18) * 89) >> 7;
  // 138: c0 50 c3
  accA += ((readMemOffs(18) * 195) >> 7) >> 8;
  // 139: 02 00 00

  // 13a: 00 00 00

  // 13b: 03 00 00

  // 13c: 04 00 00

  // 13d: 07 00 00

  // 13e: 01 00 00

  // 13f: 00 00 00

  // 140: 06 00 00

  // 141: 02 00 00

  // 142: 06 00 00

  // 143: 00 00 00

  // 144: 06 00 00

  // 145: c9 5a c0
  eramReadValue = eram[(eramPos + 7960) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA += (eramReadValue * -64) >> 7;
  accA_197 = accA;
  accB_197 = accB;

  // 146: 06 00 00

  // 147: 01 00 00
  accA_199 = accA;

  // 148: cb 50 00
  eramWriteLatch = clamp_24(accA_197);
  eram[(eramPos + 7218) & 0xffff] = eramWriteLatch >> 4;
  // 149: 04 00 00

  // 14a: 2f 75 40
  writeMemOffs(117, clamp_24(accA_199));
  accA = (readMemOffs(117) * 64) >> 7;
  // 14b: 01 fa 20
  accA += readMemOffs(122);
  accA_203 = accA;
  accB_203 = accB;

  // 14c: 02 00 00

  // 14d: 01 00 00

  // 14e: ce 50 00
  eramWriteLatch = clamp_24(accA_203);
  eram[(eramPos + 7961) & 0xffff] = eramWriteLatch >> 4;
  // 14f: 05 00 00

  // 150: 03 00 00

  // 151: 02 00 00

  // 152: 02 00 00

  // 153: 07 00 00

  // 154: 07 00 00

  // 155: 01 00 00

  // 156: 05 00 00

  // 157: 02 00 00

  // 158: c8 fa 20
  eramReadValue = eram[(eramPos + 10097) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA = (eramReadValue * 32) >> 5;
  // 159: 06 00 00

  // 15a: 02 00 00

  // 15b: 06 00 00

  // 15c: 05 00 00

  // 15d: 03 00 00

  // 15e: ca 5a c0
  eramReadValue = eram[(eramPos + 10879) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA += (eramReadValue * -64) >> 7;
  accA_222 = accA;
  accB_222 = accB;

  // 15f: 06 00 00
  accA_223 = accA;

  // 160: 00 00 00

  // 161: c8 50 00
  eramWriteLatch = clamp_24(accA_222);
  eram[(eramPos + 10098) & 0xffff] = eramWriteLatch >> 4;
  // 162: 2a 75 40
  writeMemOffs(117, clamp_24(accA_223));
  accA = (readMemOffs(117) * 64) >> 7;
  // 163: 05 fa 20
  accA += readMemOffs(122);
  // 164: 02 00 00
  accA_228 = accA;
  accB_228 = accB;

  // 165: 00 00 00

  // 166: 00 00 00

  // 167: c8 50 00
  eramWriteLatch = clamp_24(accA_228);
  eram[(eramPos + 10880) & 0xffff] = eramWriteLatch >> 4;
  // 168: 02 00 00

  // 169: 03 00 00

  // 16a: 00 00 00

  // 16b: 01 00 00

  // 16c: 05 00 00

  // 16d: 00 00 00

  // 16e: 02 00 00

  // 16f: 06 00 00

  // 170: 02 00 00

  // 171: 05 00 00

  // 172: 02 00 00

  // 173: 01 00 00

  // 174: ca fa 20
  eramReadValue = eram[(eramPos + 2627) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA = (eramReadValue * 32) >> 5;
  // 175: 06 00 00

  // 176: 05 00 00

  // 177: 04 00 00

  // 178: 00 00 00

  // 179: 02 00 00

  // 17a: ca da 20
  eramReadValue = eram[(eramPos + 5462) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA += (eramReadValue * 32) >> 5;
  // 17b: 01 00 00

  // 17c: 06 00 00

  // 17d: 02 00 00

  // 17e: 06 00 00

  // 17f: 02 00 00

  // 180: c8 da 20
  eramReadValue = eram[(eramPos + 8494) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA += (eramReadValue * 32) >> 5;
  // 181: 00 00 00

  // 182: 00 00 00

  // 183: 00 00 00

  // 184: 00 00 00

  // 185: 00 00 00

  // 186: c8 da 20
  eramReadValue = eram[(eramPos + 11441) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA += (eramReadValue * 32) >> 5;
  accA_262 = accA;

  // 187: 00 00 00

  // 188: 00 00 00

  // 189: 28 f5 20
  writeMemOffs(117, clamp_24(accA_262));
  accA = readMemOffs(117);
  accA_265 = accA;

  // 18a: 00 00 00

  // 18b: 20 7f 7f
  accA = (readMemOffs(127) * 127) >> 7;
  // 18c: 08 75 7f
  writeMemOffs(117, clamp_24(accA_265));
  accA += (readMemOffs(117) * 127) >> 7;
  accA_268 = accA;

  // 18d: 00 00 00

  // 18e: 00 00 00

  // 18f: 08 60 00
  writeMemOffs(96, clamp_24(accA_268));

  // 190: 02 00 00

  // 191: 03 00 00

  // 192: 06 00 00

  // 193: 01 00 00

  // 194: 07 00 00

  // 195: 00 00 00

  // 196: 02 00 00

  // 197: 04 00 00

  // 198: 06 00 00

  // 199: 01 00 00

  // 19a: 05 00 00

  // 19b: 01 00 00

  // 19c: ca fa 20
  eramReadValue = eram[(eramPos + 3699) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA = (eramReadValue * 32) >> 5;
  // 19d: 02 00 00

  // 19e: 03 00 00

  // 19f: 07 00 00

  // 1a0: 02 00 00

  // 1a1: 02 00 00

  // 1a2: ca da 20
  eramReadValue = eram[(eramPos + 6772) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA += (eramReadValue * 32) >> 5;
  // 1a3: 04 00 00

  // 1a4: 03 00 00

  // 1a5: 06 00 00

  // 1a6: 00 00 00

  // 1a7: 03 00 00

  // 1a8: c8 da 20
  eramReadValue = eram[(eramPos + 9690) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA += (eramReadValue * 32) >> 5;
  // 1a9: 00 00 00

  // 1aa: 00 00 00

  // 1ab: 00 00 00

  // 1ac: 00 00 00

  // 1ad: 00 00 00

  // 1ae: c8 da 20
  eramReadValue = eram[(eramPos + 12700) & 0xffff] << 4;
  writeMemOffs(122, eramReadValue);
  accA += (eramReadValue * 32) >> 5;
  accA_302 = accA;

  // 1af: 00 00 00

  // 1b0: 00 00 00

  // 1b1: 28 f5 20
  writeMemOffs(117, clamp_24(accA_302));
  accA = readMemOffs(117);
  accA_305 = accA;

  // 1b2: 00 00 00

  // 1b3: 20 7e 7f
  accA = (readMemOffs(126) * 127) >> 7;
  // 1b4: 08 75 7f
  writeMemOffs(117, clamp_24(accA_305));
  accA += (readMemOffs(117) * 127) >> 7;
  accA_308 = accA;

  // 1b5: 00 00 00

  // 1b6: 00 00 00

  // 1b7: 08 64 00
  writeMemOffs(100, clamp_24(accA_308));

  // 1b8: 20 61 85
  accA = (readMemOffs(97) * -123) >> 7;
  // 1b9: c0 50 86
  accA += ((readMemOffs(97) * 134) >> 7) >> 8;
  // 1ba: 00 e0 20
  accA += readMemOffs(96);
  // 1bb: c0 d0 00
  accA += ((readMemOffs(96) * 0) >> 7) >> 6;
  // 1bc: 00 62 7a
  accA += (readMemOffs(98) * 122) >> 7;
  // 1bd: c0 50 7a
  accA += ((readMemOffs(98) * 122) >> 7) >> 8;
  accA_317 = accA;

  // 1be: 20 62 ca
  accA = (readMemOffs(98) * -54) >> 7;
  // 1bf: c0 50 7f
  accA += ((readMemOffs(98) * 127) >> 7) >> 8;
  // 1c0: 08 e1 20
  writeMemOffs(97, clamp_24(accA_317));
  accA += readMemOffs(97);
  // 1c1: c0 d0 00
  accA += ((readMemOffs(97) * 0) >> 7) >> 6;
  // 1c2: 00 63 35
  accA += (readMemOffs(99) * 53) >> 7;
  // 1c3: c0 50 81
  accA += ((readMemOffs(99) * 129) >> 7) >> 8;
  accA_323 = accA;

  // 1c4: 00 00 00
  accA_324 = accA;

  // 1c5: 00 00 00

  // 1c6: 08 62 00
  writeMemOffs(98, clamp_24(accA_323));

  // 1c7: 08 79 00
  writeMemOffs(121, clamp_24(accA_324));

  // 1c8: 20 65 85
  accA = (readMemOffs(101) * -123) >> 7;
  // 1c9: c0 50 86
  accA += ((readMemOffs(101) * 134) >> 7) >> 8;
  // 1ca: 00 e4 20
  accA += readMemOffs(100);
  // 1cb: c0 d0 00
  accA += ((readMemOffs(100) * 0) >> 7) >> 6;
  // 1cc: 00 66 7a
  accA += (readMemOffs(102) * 122) >> 7;
  // 1cd: c0 50 7a
  accA += ((readMemOffs(102) * 122) >> 7) >> 8;
  accA_333 = accA;

  // 1ce: 20 66 ca
  accA = (readMemOffs(102) * -54) >> 7;
  // 1cf: c0 50 7f
  accA += ((readMemOffs(102) * 127) >> 7) >> 8;
  // 1d0: 08 e5 20
  writeMemOffs(101, clamp_24(accA_333));
  accA += readMemOffs(101);
  // 1d1: c0 d0 00
  accA += ((readMemOffs(101) * 0) >> 7) >> 6;
  // 1d2: 00 67 35
  accA += (readMemOffs(103) * 53) >> 7;
  // 1d3: c0 50 81
  accA += ((readMemOffs(103) * 129) >> 7) >> 8;
  accA_339 = accA;

  // 1d4: 00 00 00
  accA_340 = accA;

  // 1d5: 00 00 00

  // 1d6: 08 66 00
  writeMemOffs(102, clamp_24(accA_339));

  // 1d7: 08 77 00
  writeMemOffs(119, clamp_24(accA_340));

  // 1d8: 00 00 00

  // 1d9: 00 00 00

  // 1da: 00 00 00

  // 1db: 00 00 00

  // 1dc: 00 00 00

  // 1dd: 00 00 00

  // 1de: 00 00 00

  // 1df: 00 00 00

  // 1e0: 00 00 00

  // 1e1: 00 00 00

  // 1e2: 00 00 00

  // 1e3: 00 00 00

  // 1e4: 00 00 00

  // 1e5: 00 00 00

  // 1e6: 00 00 00

  // 1e7: 00 00 00

  // 1e8: 00 00 00

  // 1e9: 00 00 00

  // 1ea: 00 00 00

  // 1eb: 00 00 00

  // 1ec: 00 00 00

  // 1ed: 00 00 00

  // 1ee: 00 00 00

  // 1ef: 00 00 00

  // 1f0: 00 00 00

  // 1f1: 00 00 00

  // 1f2: 20 79 7f
  accA = (readMemOffs(121) * 127) >> 7;
  accA_370 = accA;

  // 1f3: c8 7e 7f
  writeMemOffs(0x7e, audioInL);
  accA = (audioInL * 127) >> 7;
  accA_371 = accA;

  // 1f4: 00 00 00

  // 1f5: 28 f5 20
  writeMemOffs(117, clamp_24(accA_370));
  accA = readMemOffs(117);
  accA_373 = accA;

  // 1f6: 28 75 7f
  writeMemOffs(117, clamp_24(accA_371));
  accA = (readMemOffs(117) * 127) >> 7;
  accA_374 = accA;

  // 1f7: 20 81 20
  accA = 128;
  // 1f8: 08 75 7f
  writeMemOffs(117, clamp_24(accA_373));
  accA += (readMemOffs(117) * 127) >> 7;
  accA_376 = accA;
  accB_376 = accB;

  // 1f9: 08 7e 00
  writeMemOffs(126, clamp_24(accA_374));

  // 1fa: 00 00 00

  // 1fb: c8 58 00
  audioOutL = clamp_24(accA_376);
  writeMemOffs(0x78, audioOutL);
  accA += (clamp_24(accA_376) * 0) >> 7;
  // 1fc: c8 6f fe
  shouldJump = true;
  // 1fd: 00 00 00

  // 1fe: 00 00 00

  // 1ff: 00 00 00

  bufferPos = (bufferPos - 1) & 0x7f;
  eramPos -= 1;
}
