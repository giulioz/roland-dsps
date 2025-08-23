# Configs

0x2000: 0xaa
0x2001: 0x00
0x2002: control pcm i/o
- 0x00: halted?
- 0x80: tr bus on, 44.1khz divider
- 0x90: tr bus off, 44.1khz divider
0x2003: host mode
- 0x54: normal write
- 0x55: masked write
- 0x56: masked write
- 0x57: read
0x2003: 0xc7
- 0x00: halted



# Instruction format overview:

bit 0-7: coef (signed 8 bit) [which can mean different things for special opcodes]
bit 8-9: shifter
- 0: >>7
- 1: >>6
- 2: >>5
- 3: >>3
bit 10-17: mem slot [which can mean different things for special opcodes]
- 1: imm 0x10
- 2: imm 0x400
- 3: imm 0x10000
- 4: imm 0x400000
- other: mem read
bit 18-22: opcode
bit 23: unused?



# TR Bus

SY is a clock with 75% low duty cycle at samplerate:  -___-___-___-___ (pins 132,133)

When SY=1 16 channels are sent, in 6bit parallel, sending 24bit samples in 4 bursts.

When SY=0 48 channels are sent, in 6bit parallel, sending 24bit samples in 4 bursts.

TR is a 6bit Parallel bus. Each sample is 4 * 6 bits.

TCK is the bit clock for the TR bus, and is 512 * sampleclock.

[e.g. for jp8000 fs = 88200, TCK = 45,158,400]



# ERAM control

ERAM bits are 23-27 in high code page (0x1000-0x1bff).

A command word is followed by 4x 5bit values, for a 20bit offset.

commands:
- 08: read @(pos + imm)
- 10: write @(pos + imm)
- 18: read @(pos + (imm & 1) + var)

var is a special register (31c-31f aka special register 7, "eram tap pos")

After a read, you can read the latch at command instruction position +0xa

Write latch can be written at command instruction position +4

# 28 bit PRAM (0x1000-0x1bff)

same as 24 bit + ERAM control

- 0 37 28 00   write to host reg // Special register write. Unimaginably useful.

These can be easily understood as writes to GRAM area for audio out.
- 0 39 c0 00   output DA3 (accA)
- 0 3a 2c 00   output DA3 (accA)
- 0 3a 3c 00   output DA3 (accA)
- 0 3a ac 00   output DA3 (accA)
- 0 3a 18 00   output DA3 (accA)




# GRAM Instructions

- 3a 40 00  -> write accA into GRAM [opc: 38, imm = 0, mem = 90. Write accA into GRAM[0x90]]
- 3c 40 00  -> write accB into GRAM [opc: 38, imm = 0, mem = 90. Write accB into GRAM[0x90]]
- 20 54 40  -> sum into accA from GRAM [opc: 20, imm = 0x40, mem = 0x15. accA += GRAM[0x15] * 0x40 >> 7]
- 24 54 40  -> load into accA from GRAM [opc: 24, imm = 0x40, mem = 0x15. accA = GRAM[0x15] * 0x40 >> 7]

# GRAM Address Space (measured with TR off)

- 0x00-0x5f first area (delay line)
- 0x60-0x6f weird area where only pairs work as delay line? (audio input?) // Maybe TR is here? *mysterious*
- 0x70-0xe6 audio out area? // Maybe TR is here?
- 0xe7-0xff normal delay line?

-> NOTE: those addresses change if you enable the TR bus
  0x70-0xff: free?
  0x80-0xdf: TR bus out


0xe6 is where the audio "goes out"?
  -> writing to 0x70+ then to 0xe6 works for LR output
  -> writing anywhere else instead of e6 outputs that value on both L and R

If you do not include a write to e6 then you get mono out from the highest address written to



# Special Regs

special regs for opcode 34:

mem & 0xf0 == 0xa0.

- bytes   (mem): desc
- 36 80 00 (a0): write mul coef 0 from accA (28 bit pgm only)
- 36 84 00 (a1): write mul coef 0 from accB (28 bit pgm only)
- 36 88 00 (a2): write mul coef 1 from accA (28 bit pgm only)
- 36 8c 00 (a3): write mul coef 1 from accB (28 bit pgm only)
- 36 90 00 (a4): write mul coef 2 from accA (28 bit pgm only)
- 36 94 00 (a5): write mul coef 2 from accB (28 bit pgm only)
- 36 98 00 (a6): write mul coef 3 from accA (24 bit pgm only)
- 36 9c 00 (a7): write mul coef 3 from accB (24 bit pgm only)
- 36 a0 00 (a8): write mul coef 4 from accA (24 bit pgm only)
- 36 a4 00 (a9): write mul coef 4 from accB (24 bit pgm only)
- 36 a8 00 (aa): write mul coef 5 from accA (24 bit pgm only)
- 36 ac 00 (ab): write mul coef 5 from accB (24 bit pgm only)
- 36 b0 00 (ac): write ? // *mysterious* - these may be entirely unimplemented.
- 36 b4 00 (ad): write ?
- 36 b8 00 (ae): write ?
- 36 bc 00 (af): write ?

mem & 0xf0 >= 0xc0.

- 300->33c: store and sum accA
- 340->37c: store and replace accA
- 380->3bc: store and sum accB
- 3c0->3fc: store and replace accB

- bytes [nibble 3 may be +0-3]
- 37 00 ?? / 37 40 ?? / 37 80 ?? / 37 c0 ??   (c/d/e/f 0): jump to shifter+coef if acc==0
- 37 04 ?? / 37 44 ?? / 37 84 ?? / 37 c4 ??   (c/d/e/f 1): jump to shifter+coef if acc<0
- 37 08 ?? / 37 48 ?? / 37 88 ?? / 37 c8 ??   (c/d/e/f 2): jump to shifter+coef if acc>=0  (jump has 2 delay slots)
- 37 0c ?? / 37 4c ?? / 37 8c ?? / 37 cc ??   (c/d/e/f 3): jump to shifter+coef always?
- 37 10 ?? / 37 50 ?? / 37 90 ?? / 37 d0 ??   (c/d/e/f 4): reads iram?  write?
- 37 14 ?? / 37 54 ?? / 37 94 ?? / 37 d4 ??   (c/d/e/f 5): reads iram?  write?
- 37 18 ?? / 37 58 ?? / 37 98 ?? / 37 d8 ??   (c/d/e/f 6): read previous mac mem input >> 7
- 37 1c ?? / 37 5c ?? / 37 9c ?? / 37 dc ??   (c/d/e/f 7): reads iram?  eram tap pos?
- 37 20 ?? / 37 60 ?? / 37 a0 ?? / 37 e0 ??   (c/d/e/f 8): reads iram?  write?
- 37 24 ?? / 37 64 ?? / 37 a4 ?? / 37 e4 ??   (c/d/e/f 9): reads iram?  write?
- 37 28 ?? / 37 68 ?? / 37 a8 ?? / 37 e8 ??   (c/d/e/f a): reads iram?  write host interface accA
- 37 2c ?? / 37 6c ?? / 37 ac ?? / 37 ec ??   (c/d/e/f b): reads iram?  eram write latch?
- 37 30 ?? / 37 70 ?? / 37 b0 ?? / 37 f0 ??   (c/d/e/f c): read 0?   eram read latch?
- 37 34 ?? / 37 74 ?? / 37 b4 ?? / 37 f4 ??   (c/d/e/f d): read 0?   eram read latch?
- 37 38 ?? / 37 78 ?? / 37 b8 ?? / 37 f8 ??   (c/d/e/f e): read 0?   eram read latch?
- 37 3c ?? / 37 7c ?? / 37 bc ?? / 37 fc ??   (c/d/e/f f): read 0?   eram read latch?

* We do not expect to see mem < 0xa0 or (mem & 0xf0) == 0xb0.


# Opcodes!

store
- MAC A
  - 00: sum accA { accA += coeff * mem >> shift; }
  - 04: load accA { accA = coeff * mem >> shift; }
  - 08: store accA sat to iram, then accA=stored val { iram[mem] = sat(accA(pc - 3)); accA = coeff * accA(pc - 3) >> shift; }
  - 0c: store accB sat to iram, then accA=stored val { iram[mem] = sat(accB(pc - 3)); accA = coeff * accB(pc - 3) >> shift; }

- MAC B
  - 10: sum accB { accB += coeff * mem >> shift; }
  - 14: load accB { accB = coeff * mem >> shift; }
  - 18: store accA sat to iram, then accB=stored val { iram[mem] = sat(accA(pc - 3)); accB = coeff * accA(pc - 3) >> shift; }
  - 1c: store accB sat to iram, then accB=stored val { iram[mem] = sat(accB(pc - 3)); accB = coeff * accB(pc - 3) >> shift; }

- SPECIAL/MUL/GRAM
  - 20: sum gram -> accA // See GRAM Instructions.
    - seems to be using 2 pipeline slots before (or after?) = lots of NOPs seem to go around these instructions.
  - 24: load gram -> accA // See GRAM Instructions.
  - 28: normal accA load? (unused? none in the code)
  - 2c: normal accA load? (unused? none in the code)
    - those do not store anything, nor they do absolute value
  - 30: mult per variable (see below) // see mul opcode 30 coefs. Use a special register as the multiplier coefficient. mem works like normal, but coeff is setup.
  - 34: special registers read/write (see below, also Special Regs above)
  - 38: gram write accA // see GRAM coefficients
  - 3c: gram write accB // see GRAM coefficients
    - coef always zero but shift varies? *mysterious* - let's check if this shifts the written value.

- UNSAT/CLAMP
  - 40: store accA to iram unsat, then accA+=stored val { iram[mem] = accA(pc - 3); accA += coeff * accA(pc - 3) >> shift; }
  - 44: store accA to iram unsat, then accA=stored val { iram[mem] = accA(pc - 3); accA = coeff * accA(pc - 3) >> shift; }
  - 48: store accA to iram clamp neg to 0, then accA+=stored val { iram[mem] = max(0, accA(pc - 3)); accA += coeff * accA(pc - 3) >> shift; }
  - 4c: store accA to iram clamp neg to 0, then accA=stored val { iram[mem] = max(0, accA(pc - 3)); accA = coeff * accA(pc - 3) >> shift; }
  NOTE: those then load into accA the clamped/unsat value

- ?
  - 50: load accA, following instructions conditionals (see "conditional opcode 50" below)
  - 54: sum accA?
    - seems identical to 00
    - used in the jp8000 only? *mysterious*
    - doesn't seem to be an abs value either
  - 58: store accA to iram, then accA+=stored val { iram[mem] = sat(accA(pc - 3)); accA += coeff * accA(pc - 3) >> shift; } // Is accA(pc-3) in the second term saturated or unsaturated???
  - 5c: store accB to iram, then accB+=stored val { iram[mem] = sat(accB(pc - 3)); accB += coeff * accB(pc - 3) >> shift; }

- ?
  - 60: accA += (~mem & 0x7fffff) * coeff >> shift (unused?)
  - 64: accA = (~mem & 0x7fffff) * coeff >> shift (unused?)
  - 68: store accA to iram, then accA += (accA(pc - 3) & 0x7fffff) * coeff >> shift
  - 6c: store accA to iram, then accA = (accA(pc - 3) & 0x7fffff) * coeff >> shift

- ?
  - 70: accA += (~mem & 0x7fffff) * coeff >> shift (unused?)
  - 74: accA = (~mem & 0x7fffff) * coeff >> shift (unused?)
  - 78: store accA to iram, then accA += (~accA(pc - 3) & 0x7fffff) * coeff >> shift
  - 7c: store accA to iram, then accA = (~accA(pc - 3) & 0x7fffff) * coeff >> shift


opcode=0x64 acc=0x45<<3( 552) -> 0x7ffdd7   ~acc & 0x7fffff
opcode=0x64 acc=0x00<<3( 000) -> 0x7fffff   ~acc & 0x7fffff
opcode=0x64 acc=0xc5<<3(-472) -> 0x0001d7   ~acc & 0x7fffff

opcode=0x68 acc=0x45<<3( 552) -> 0x7fffff    acc + (~acc & 0x7fffff)
opcode=0x68 acc=0x00<<3( 000) -> 0x7fffff    acc + (~acc & 0x7fffff)
opcode=0x68 acc=0xc5<<3(-472) -> 0x7ffc50    acc + ( acc & 0x7fffff)

opcode=0x6c acc=0x45<<3( 552) -> 0x7ffdd7   ~acc & 0x7fffff
opcode=0x6c acc=0x00<<3( 000) -> 0x7fffff   ~acc & 0x7fffff
opcode=0x6c acc=0xc5<<3(-472) -> 0x7ffe28    acc & 0x7fffff

opcode=0x74 acc=0x45<<3( 552) -> 0x000228   ~acc & 0x7fffff
opcode=0x74 acc=0x00<<3( 000) -> 0x000000   ~acc & 0x7fffff
opcode=0x74 acc=0xc5<<3(-472) -> 0x0001d7   ~acc & 0x7fffff

opcode=0x78 acc=0x45<<3( 552) -> 0x000450    acc + ( acc & 0x7fffff)
opcode=0x78 acc=0x00<<3( 000) -> 0x000000    acc + ( acc & 0x7fffff)
opcode=0x78 acc=0xc5<<3(-472) -> 0xffffff    acc + (~acc & 0x7fffff)

opcode=0x7c acc=0x45<<3( 552) -> 0x000228    acc & 0x7fffff
opcode=0x7c acc=0x00<<3( 000) -> 0x000000    acc & 0x7fffff
opcode=0x7c acc=0xc5<<3(-472) -> 0x0001d7   ~acc & 0x7fffff


# conditional opcode 50:
- pc : note
- +0 opcode 50
- +1  always execute
- +2  always execute
- +3  always execute
- +4  always execute
- +5 executed only if result (mem * coef) is < 0
- +6 executed only if result (mem * coef) is < 0
- +7 executed only if result (mem * coef) is >= 0
- +8 executed only if result (mem * coef) is >= 0
- +9 executed only if result (mem * coef) is >= 0
- +a executed only if result (mem * coef) is >= 0

--> NOTE: this seems to not be holding true for everything, like for example doing an accA replace will still set it to 0. It seems like what this is going is actually just gating the access to the memory, so that if the condition is not fulfilled, it loads 0 instead.


# mul opcode 30 coefs:
- bit 0: replace/sum
- bit 1: dest accA/accB
- bit 2: input is memory / input is accA
- bit 3: negate coefficient
- bit 4: unknown - does something to the coefficient *mysterious*
- bit 5-7: select coefficient source [mul coef 0, ..., mul coeff 5, ERAM pos, ERAM pos]

examples

- 00: coef=(mul coef 0)>>16, val=mem, result replaces accA
- 01: coef=(mul coef 0)>>16, val=mem, result sums accA
- 02: coef=(mul coef 0)>>16, val=mem, result replaces accB
- 03: coef=(mul coef 0)>>16, val=mem, result sums accB
- 04: coef=(mul coef 0)>>16, val=accA, result replaces accA
- 05: coef=(mul coef 0)>>16, val=accA, result sums accA
- 06: coef=(mul coef 0)>>16, val=accA, result replaces accB
- 07: coef=(mul coef 0)>>16, val=accA, result sums accB
- 08: coef=-(mul coef 0)>>16, val=mem, result replaces accA
- 09: coef=-(mul coef 0)>>16, val=mem, result sums accA
- 0a: coef=-(mul coef 0)>>16, val=mem, result replaces accB
- 0b: coef=-(mul coef 0)>>16, val=mem, result sums accB
- 0c: coef=-(mul coef 0)>>16, val=accA, result replaces accA
- 0d: coef=-(mul coef 0)>>16, val=accA, result sums accA
- 0e: coef=-(mul coef 0)>>16, val=accA, result replaces accB
- 0f: coef=-(mul coef 0)>>16, val=accA, result sums accB

- 10: coef?? (related to mul coef 0), val=mem, result replaces accA
- 11: coef?? (related to mul coef 0), val=mem, result sums accA
- 12: coef?? (related to mul coef 0), val=mem, result replaces accB
- 13: coef?? (related to mul coef 0), val=mem, result sums accB
- 14: coef?? (related to mul coef 0), val=accA, result replaces accA
- 15: coef?? (related to mul coef 0), val=accA, result sums accA
- 16: coef?? (related to mul coef 0), val=accA, result replaces accB
- 17: coef?? (related to mul coef 0), val=accA, result sums accB
- 18: coef=?? (related to mul coef 0), val=mem, result replaces accA
- 19: coef=?? (related to mul coef 0), val=mem, result sums accA
- 1a: coef=?? (related to mul coef 0), val=mem, result replaces accB
- 1b: coef=?? (related to mul coef 0), val=mem, result sums accB
- 1c: coef=?? (related to mul coef 0), val=accA, result replaces accA
- 1d: coef=?? (related to mul coef 0), val=accA, result sums accA
- 1e: coef=?? (related to mul coef 0), val=accA, result replaces accB
- 1f: coef=?? (related to mul coef 0), val=accA, result sums accB

- 2x/3x: mul coef 1
- 4x/5x: mul coef 2
- 6x/7x: mul coef 3 (24 bit pgm)
- 8x/9x: mul coef 4 (24 bit pgm)
- ax/bx: mul coef 5 (24 bit pgm)
- cx/dx: interpolation coef? ERAM pos? *mysterious*
- ex/fx: interpolation coef? 1 - ERAM pos? *mysterious*


## ooooooold.

# host reg writes
- b7 28 00   write to host reg
- 37 28 00   write to host reg  accA?
- 37 a8 00   write to host reg  accB?
# regular macs
- 04 10 12   load 12<<15   accA?
- 14 04 5f   load 5f>>3??  accB?
- 94 04 32   load 32>>3??

# immediate loads?
- 14 04 32   load 32>>3??
- 14 05 32   load 32>>2??
- 14 06 32   load 32>>1??
- 14 07 32   load 32<<1??

- 14 08 32   load 32<<3??
- 14 09 32   load 32<<4??
- 14 0a 32   load 32<<5??
- 14 0b 32   load 32<<7??


- 14 0c 32   load 32<<9??
- 14 0d 32   load 32<<10??
- 14 0d 32   load 32<<11??
- 14 0f 32   load 32<<13??


- 14 10 32   load 32<<15??
- 14 11 32   load 32<<16??
- 14 12 32   load 32<<17??
- 14 13 32   load 32<<19??


# Typical MUL instructions
- 04 00 7f   accA = (mem[??] * 7f) >> 7
- 04 01 7f   accA = (mem[??] * 7f) >> 6
- 04 02 7f   accA = (mem[??] * 7f) >> 5
- 04 03 7f   accA = (mem[??] * 7f) >> 3
- 04 04 7f   accA = (imm 0x7f * 0x10) >> 7
- 04 05 7f   accA = (imm 0x7f * 0x10) >> 6
- 04 06 7f   accA = (imm 0x7f * 0x10) >> 5
- 04 07 7f   accA = (imm 0x7f * 0x10) >> 3
- 04 08 7f   accA = (imm 0x7f * 0x400) >> 7
- 04 09 7f   accA = (imm 0x7f * 0x400) >> 6
- 04 0a 7f   accA = (imm 0x7f * 0x400) >> 7
- 04 0b 7f   accA = (imm 0x7f * 0x400) >> 3
- 04 0c 7f   accA = (imm 0x7f * 0x10000) >> 7
- 04 0d 7f   accA = (imm 0x7f * 0x10000) >> 6
- 04 0e 7f   accA = (imm 0x7f * 0x10000) >> 5
- 04 0f 7f   accA = (imm 0x7f * 0x10000) >> 3
- 04 10 7f   accA = (imm 0x7f * 0x400000) >> 7
- 04 11 7f   accA = (imm 0x7f * 0x400000) >> 6
- 04 12 7f   accA = (imm 0x7f * 0x400000) >> 5
- 04 13 7f   accA = (imm 0x7f * 0x400000) >> 3
