# Question marks
- Width of accumulators
- Width of IRAM (30 vs 24 bit)
- IRAM addressing mode (always circular buffer?)
- Config registers
- Recheck order of operations/signs on opcodes 4/5/6/7/8/9 (test with script)
- Protocol for TRR/TRS
- DRAM control
- Opcode 0 bit 2 (SE-70 only)
- Store x2 (constant?)
- Many special regs
- Params opcode c/d/e/f


# From SRV330

CRAM: 0x0000-0x07ff
PRAM: 0x1000-0x1fff

IRAM (30 bit??)


## Read
0000: readback
0001: readback
0002: readback
0003: status

## Write
0802: readback?

0804: config
  0x40: 12 bit ram
0805: config
0806: config

0808: config?
0809: config?
080a: config?

## Read PRAM
SD330: csp_r 000003
SD330: csp_w 000804=47
SD330: csp_w 000805=80
SD330: csp_w 000806=02
SD330: csp_r 000003

SD330: csp_r 000003
SD330: csp_w 003002=00
SD330: csp_r 000003
SD330: csp_r 000000
SD330: csp_r 000001
SD330: csp_r 000002

SD330: csp_r 000003
SD330: csp_w 003006=00
SD330: csp_r 000003
SD330: csp_r 000000
SD330: csp_r 000001
SD330: csp_r 000002



## Pins
102 MSF: MSB first (32 bit LE/BE)
66 SYI: sample rate
133 TRCKIN: sample rate * 24 * 4 (24 channels x 24 bit?)

## I/O

TRS/TRR is kinda parallel/serial.
TRS regs 1b0-1bf when SY is up
TRS regs 1c0-1cf when SY is down




## Instructions

acc is 38 bits?

00 x4 8a 0000   -> no write
00 x5 8a 0000   -> write to temp reg?   2 slots pipeline before!
00 x6 8a 0000   -> no write
00 x7 8a 0000   -> write to temp reg?   "
00 x8 8a 0000   -> no write
00 x9 8a 0000   -> no write
00 xa 8a 0000   -> no write

00 30 01 01f0   -> load 01f0 << 0
00 31 01 01f0   -> load 0000 ??
00 32 01 01f0   -> load 01f0 << 0
00 33 01 01f0   -> load (0x10000 - (01f0 << 0)) << 8 ??
00 34 01 01f0   -> load 01f0 << 0
00 35 01 01f0   -> load (0x10000 - (01f0 << 0)) << 8 ??
00 36 01 01f0   -> load 01f0 << 0
00 37 01 01f0   -> load (0x10000 - (01f0 << 0)) << 8 ??
00 38 01 01f0   -> load 01f0 << 0
00 39 01 01f0   -> load ??
00 3a 01 01f0   -> load 01f0 << 0
00 3b 01 01f0   -> load 0000 ??
00 3c 01 01f0   -> load 01f0 << 0
00 3d 01 01f0   -> load ??
00 3e 01 01f0   -> load 01f0 << 0
00 3f 01 01f0   -> load 0000 ??

00 3c 02 01f0   -> load 01f0 << 7
00 3c 03 01f0   -> load mem[03] * 0x1f0 ??
00 10 01 0100   -> sum 0100
00 10 02 0100   -> sum 0100 << 7
00 10 03 0100   -> sum mem[03] * 0x100 ??

01 27 93 4000   -> load audio in


00 20 01 2f00   -> load 0x2f00
00 00 00
00 00 00
00 0c 15        -> store to mem[15]
00 30 15 7fff   -> load mem[15] * 0x7fff
00 00 00
00 00 00
00 07 8a        -> store to temp reg?



ab cd ef
  a: ram control?
  b: shifter?
    0: 0 (shifter=15)
    1: 1 (shifter=14)
    2: 2 (shifter=13)
    3: 4 (shifter=11)
    ...
  c: opcode
  d: store
  ef: ram slot/shifter for coef?


d (store)
  x0 (0)  (no store)
  x2 (1)  store/load const 0x800000? (related to special 177)
  x4 (2)  store accB special reg? unsaturated
  x6 (3)  store accB special reg? saturated
  x8 (4)  store accA unsaturated
  xa (5)  store accB unsaturated
  xc (6)  store accA saturated
  xe (7)  store accB saturated


c (opcode)
  fx special reg -> accB?  always has small coefs
  ex nothing?  special reg -> accA?  always has small coefs
  dx something invert? replaces accB, always has small coefs
  cx nothing? replaces accA, always has small coefs
  
  bx nothing?
  ax nothing? accA LSB adjust?
  9x accA += ? like 8x but increment
  8x accA  = ? like 2x but different sign (abs?)
  7x accA += ? similar to 6x but increment (unused?)
  6x accA  =
    val pos: 0x800000 - (coef/mem) - 1
    val neg: (coef/mem) | 0x7f0000 ??
  5x accA = max(0, (mul >=0 ? mul : ~mul) + accA) (requires 3 pipeline slots instead of 2? otherwise 00)
  4x same as 5x but no accumulate? (unused?)
  3x accB  = mul >> shift
  2x accA  = mul >> shift
  1x accB += mul >> shift
  0x accA += mul >> shift


## Shifter (const)
00: (mem)
01: << 0
02: << 7
03: (mem)
...


## Special regs
004: ?
006: ?
007: ?


unsat:
176: jump
185: mult coef for 07? (one per dram tap? address?)
18d: mult coef for 07? (used in se70 rotary/pitch shift)
1c2: ? only in 2nd chip intro

sat:
172: jump (always?)
174: jump
175: jump
176: jump
177: jump

182: ? mostly 2nd chip intro
183: ? something ram?
186: ?
187: mult coef for 07?
18a: host interface
18b: ?
18e: ?
18f: mult coef for 07 (only se70)

192: ? only se70
193: ? sde/srv intro (audio?)
194: ? only in 2nd chip intro
195: ? only in 2nd chip intro

1a0: ? only in srv 1/2 chip intro
1a1: ? only in srv 1/2 chip intro
1a2: ?
1a3: ? only in srv 1/2 chip intro
1a4: ? only in srv 1/2 chip intro
1a5: ? only in srv 1/2 chip intro
1a7: ? sde/srv first instruction each chip
1a8: ? only in 2nd chip intro
1aa: ? only srv 1st chip
1ac: ? only se70

1b0: ? only sde/srv outro
1b2: ? only se70 test
1bb: ? only se70 test
1bc: ? only se70 test/sampler

1c4: ? only sde/srv outro (audio?)
1c7: ? only srv 1st chip intro
1c8: ? only se70(some)/srv intro
1ca: ? only srv 2st chip intro
1cb: ? only se70 intro
1cc: ? only se70

1d2: ? only se70
1d4: ? only srv 1st chip outro
1db: ? only se70 intro
1dc: ? only se70

1e0: ? only in srv 1/2 chip
1e1: ? only in srv 1/2 chip
1e3: ? only in srv 1/2 chip
1e4: ? only in srv 1/2 chip
1e5: ? only in srv 1/2 chip

1f0: dram write?
1f1-1ff: dram taps?


18b-18c
18d-18e


## DRAM

Only highest nibble controls offset
x8 bit starts operation

Sequence:
+0 command
+1 adjust << 0
+2 adjust << 4
+3 adjust << 8
+4 adjust << 12
+5 adjust << 16

Essentially composing 5 nibbles, then ANDing 0x3ffff (24 bit).

s183 is the latch for write (has to be written before or on pos +5).

s1f0-1ff are the read latches (how do they work?) and can be read at position +a.

### Commands

00
08 read
10
18 read
20
28 read
30
38 read

40
48 write special 183 (pos +5)
50
58 write
60
68 write
70
78 write

80
88 read absolute s185
90
98 read
a0
a8 read
b0
b8 read
c0
c8 read
d0
d8 read
e0
e8 read
f0
f8 read





12: write?
32: read indirect?


### 12 write
+0: 12
+4: write special 185


### Read direct
+0: 32
+1: (adjust)
+2: (adjust)
+3: (adjust)
+b: read tap


+0  addr LSB
+1  addr
+2  addr MSB
+8  read tap (xx 27 fx)



### Offsetting (12 bit)
08 - 10   -> read col +02 (12 bit)


08 - 00 - 10   -> read col +20
08 - 00 - 20   -> read col +40
08 - 00 - 40   -> read col +80
08 - 00 - 60   -> read col +c0
08 - 00 - 80   -> read row +01
08 - 00 - 90   -> read row +01 col +20

08 - 00 - 00 - 10   -> read row +02
08 - 00 - 00 - 14   -> read row +02
08 - 00 - 00 - 18   -> read row +02 (mess up read)
08 - 00 - 00 - 20   -> read row +04
08 - 00 - 00 - 30   -> read row +06
08 - 00 - 00 - 40   -> read row +08
08 - 00 - 00 - d0   -> read row +1a
08 - 00 - 00 - e0   -> read row +1c
08 - 00 - 00 - f0   -> read row +1e

08 - 00 - 00 - 00 - 10   -> read col +100
08 - 00 - 00 - 00 - 20   -> (no change)
08 - 00 - 00 - 00 - 30   -> read col +100


### Offsetting (24 bit)
f8    -> read col -01

08 - 04    -> (no change)




### Starts

08 read





## opcodes
0 accA incr
1 accB incr
2 accA repl
3 accB repl

4 accA repl
5 accA incr
6 accA repl
7 accA incr
8 accA repl
9 accA incr

accA:a000  mem:1000  coef:4000
0: ffb000   -24576 + 4096
2: 001000   4096
5: 000000   max(0, -24576 + 4096)
6: ffefff   ??
7: ff8fff
8: 001000
9: ffb000

accA:a000  mem:1000  coef:a000
0: ff8800
2: ffe800
4: 0017ff
5: 000000
6: ffe800
7: ff8800
8: 0017ff
9: ffb7ff



opcode 5:
accA:7000  mem:1234  coef:4000  -> 008234   max(0, abs(mult)-1+accA)
accA:2000  mem:1234  coef:4000  -> 003234   max(0, abs(mult)-1+accA)
accA:0200  mem:1234  coef:4000  -> 001434   max(0, abs(mult)-1+accA)
accA:0000  mem:1234  coef:4000  -> 001234   max(0, abs(mult)-1+accA)

accA:ff00  mem:1234  coef:4000  -> 001134   max(0, abs(mult)-1+accA)
accA:f000  mem:1234  coef:4000  -> 000234   max(0, abs(mult)-1+accA)
accA:ef00  mem:1234  coef:4000  -> 000134   max(0, abs(mult)-1+accA)
accA:ee00  mem:1234  coef:4000  -> 000034   max(0, abs(mult)-1+accA)
accA:ea00  mem:1234  coef:4000  -> 000000   max(0, abs(mult)-1+accA)
accA:e000  mem:1234  coef:4000  -> 000000   max(0, abs(mult)-1+accA)
accA:a000  mem:1234  coef:4000  -> 000000   max(0, abs(mult)-1+accA)
accA:8000  mem:1234  coef:4000  -> 000000   max(0, abs(mult)-1+accA)


accA:0000  mem:1234  coef:a000  -> 001b4d

accA:0000  mem:1234 (4660)    coef:4000  -> 001234
accA:0000  mem:f234 (-3532)   coef:4000  -> 000dcb   abs(mult)-1
accA:0000  mem:8234 (-32204)  coef:4000  -> 007dcb

accA:0000  mem:4321 (17185)  coef:a000 (-24576)  -> 007dcb


opcode a:

00 a0 02 4000  -> accA += 0x4000 >> 8
01 a0 02 4081  -> accA += 0x4081 >> 7
01 a0 02 f000  -> accA += -4096 >> 8
00 a0 01 4000  -> accA += 0x4000 >> 15
00 a0 01 8000  -> accA += -32768 >> 15
00 a0 70 4000  -> accA += (0x91a00//2) >> 15  (mem=0x91a00)

a: accA += (mem[offs] * coef) >> 15 >> shift
b: accB += (mem[offs] * coef) >> 15 >> shift



opcode c/d/e/f:
param 0:
param 1:
param 2:
param 3:
param 4:
param 5:
param 6:
param 7: mult



## Jump
No delay slot?
coef is dest (as-is)
uses pipeline (-3) value to check

special reg 172








# CSP
ERAM (DRAM)
PRAM
CRAM
IRAM

## Write
1001-1fff  IRAM (24 bit, 4 each, skip %4==0)
0000-07ff  PRAM (16 bit, 2 each)
  0000-0089: ?
  008a-07ff: ?
0800-0805  Config
  0803 readback internal reg?
  0804 readback internal reg?
  0805
  0806
  0807
  080b
2803 readback?

2000-27ff read sel PRAM (+1 => 0x00,0x01)
2800-2fff read sel ??
3000-3fff read sel IRAM (+2 => 0x01,0x02  16 bit only?)

## Read
0000 status
0001
0002
0003


## Instructions

instr[0][7:4]: ram control?
  1: read?  then 3 nibbles (offset)
  b: read?  then 3 nibbles (offset)
  4: ??     then 4 nibbles (??)
  c: write?
