# Configs

0x2000: 0xaa
0x2001: 0x00
0x2002: control pcm i/o
  0x00: halted?
  0x80: tr bus on, 44.1khz divider
  0x90: tr bus off, 44.1khz divider
0x2003: host mode
  0x54: normal write
  0x55: masked write
  0x56: masked write
  0x57: read
0x2003: 0xc7
  0x00: halted



# TR Bus
16 channels when SY=1, then 48 channels when SY=0?


# ERAM control

commands:
- 08: read @(pos + imm)
- 10: write @(pos + imm)
- 18: read @(pos + (imm & 1) + var)




# 28 bit PRAM

same as 24 bit + ram control?

0 37 28 00   write to host reg

0 39 c0 00   output DA3 (accA)
0 3a 2c 00   output DA3 (accA)
0 3a 3c 00   output DA3 (accA)
0 3a ac 00   output DA3 (accA)
0 3a 18 00   output DA3 (accA)


# 24 bit PRAM

special regs 38:

- 1c0-1f8: audio out
- 200-2f8: audio out
- 300-380: audio out



special regs 34:

- 280: write? reads iram
- 284: write? reads iram
- 288: write? reads iram
- 28c: write? reads iram
- 290: write? reads iram
- 294: write? reads iram
- 298: write? reads iram
- 29c: write? reads iram
- 2a0: write? reads iram
- 2a4: write? reads iram
- 2a8: write? reads iram
- 2ac: write? reads iram

300->33c: store and sum accA
340->37c: store and replace accA
380->3bc: store and sum accB
3c0->3fc: store and replace accB

- 300 (c/d/e/f 0): reads iram?  write?
- 304 (c/d/e/f 1): reads iram?  write?
- 308 (c/d/e/f 2): jump to shifter+coef if acc<0  (jump has 2 delay slots)
- 30c (c/d/e/f 3): jump to shifter+coef always?
- 310 (c/d/e/f 4): reads iram?  write?
- 314 (c/d/e/f 5): reads iram?  write?
- 318 (c/d/e/f 6): read previous mac result >> 7
- 31c (c/d/e/f 7): reads iram?  eram tap pos?
- 320 (c/d/e/f 8): reads iram?  write?
- 324 (c/d/e/f 9): reads iram?  write?
- 328 (c/d/e/f a): reads iram?  write host interface accA
- 32c (c/d/e/f b): reads iram?  eram write latch?
- 330 (c/d/e/f c): read 0?   eram read latch?
- 334 (c/d/e/f d): read 0?   eram read latch?
- 338 (c/d/e/f e): read 0?   eram read latch?
- 33c (c/d/e/f f): read 0?   eram read latch?

bit 0-7: coef (signed 8 bit)
bit 8-9: shifter
  0: >>7
  1: >>6
  2: >>5
  3: >>3
bit 10-17: mem slot
  1: imm 0x10
  2: imm 0x400
  3: imm 0x1000
  4: imm 0x400000
  other: mem read
bit 18-22: opcode
bit 23: unused?

store
- 34: store special

- 00: sum accA
- 04: load accA
- 08: store accA sat to iram, then accA=stored val
- 0c: store accB sat to iram, then accA=stored val

- 10: sum accB
- 14: load accB
- 18: store accA sat to iram, then accB=stored val
- 1c: store accB sat to iram, then accB=stored val

- 20: sum accA inverted?  ((~mem >> 2) * coef) >> shift
- 24: load accA inverted?  ((~mem >> 2) * coef) >> shift
- 28: normal accA load?
- 2c: normal accA load?

- 30: sum accA inverted?  ((~mem >> 2) * coef) >> shift
- 34: normal accA sum? (store special?)
- 38: normal accA sum? (store audio out?)
- 3c: normal accA sum?

- 40: store accA to iram unsat, then accA+=stored val
- 44: store accA to iram unsat, then accA=stored val
- 48: store accA to iram clamp neg to 0, then accA+=stored val
- 4c: store accA to iram clamp neg to 0, then accA=stored val

- 50: load accA, following instructions conditionals (see CSP)
- 54: sum accA, following instructions conditionals (see CSP)
- 58: store accA to iram, then accA+=stored val
- 5c: store accB to iram, then accB+=stored val

- 60: accA += something like 0x7fffff - mem?
- 64: accA = something like 0x7fffff - mem?
- 68: store accA to iram, then accA += something like 0x7fffff - stored val?
- 6c: store accA to iram, then accA = something like 0x7fffff - stored val?

- 70: accA += something like 0x7fffff - mem?
- 74: accA = something like 0x7fffff - mem?
- 78: store accA to iram, then accA += something like 0x7fffff - stored val?
- 7c: store accA to iram, then accA = something like 0x7fffff - stored val?

opcodes:
- 00: sum accA
- 04: load accA
- 10: sum accB
- 14: load accB
- 20: sum and invert (pre and shift 1??) accA
- 24: load and invert accA


b7 28 00   write to host reg
37 28 00   write to host reg  accA?
37 a8 00   write to host reg  accB?

04 10 12   load 12<<15   accA?

14 04 5f   load 5f>>3??  accB?

94 04 32   load 32>>3??

14 04 32   load 32>>3??
14 05 32   load 32>>2??
14 06 32   load 32>>1??
14 07 32   load 32<<1??

14 08 32   load 32<<3??
14 09 32   load 32<<4??
14 0a 32   load 32<<5??
14 0b 32   load 32<<7??

14 0c 32   load 32<<9??
14 0d 32   load 32<<10??
14 0d 32   load 32<<11??
14 0f 32   load 32<<13??

14 10 32   load 32<<15??
14 11 32   load 32<<16??
14 12 32   load 32<<17??
14 13 32   load 32<<19??


04 00 7f   accA = (mem[??] * 7f) >> 7
04 01 7f   accA = (mem[??] * 7f) >> 6
04 02 7f   accA = (mem[??] * 7f) >> 5
04 03 7f   accA = (mem[??] * 7f) >> 3
04 04 7f   accA = (imm 0x7f * 0x10) >> 7
04 05 7f   accA = (imm 0x7f * 0x10) >> 6
04 06 7f   accA = (imm 0x7f * 0x10) >> 5
04 07 7f   accA = (imm 0x7f * 0x10) >> 3
04 08 7f   accA = (imm 0x7f * 0x400) >> 7
04 09 7f   accA = (imm 0x7f * 0x400) >> 6
04 0a 7f   accA = (imm 0x7f * 0x400) >> 7
04 0b 7f   accA = (imm 0x7f * 0x400) >> 3
04 0c 7f   accA = (imm 0x7f * 0x1000) >> 7
04 0d 7f   accA = (imm 0x7f * 0x1000) >> 6
04 0e 7f   accA = (imm 0x7f * 0x1000) >> 5
04 0f 7f   accA = (imm 0x7f * 0x1000) >> 3
04 10 7f   accA = (imm 0x7f * 0x400000) >> 7
04 11 7f   accA = (imm 0x7f * 0x400000) >> 6
04 12 7f   accA = (imm 0x7f * 0x400000) >> 5
04 13 7f   accA = (imm 0x7f * 0x400000) >> 3
