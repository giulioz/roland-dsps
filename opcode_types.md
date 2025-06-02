## Instructions 00-7f

00 01 cc  accA = accA + cc
00 02 cc  accA = accA + (cc << 5)
00 03 cc  accA = accA + (cc << 10)
00 04 cc  accA = accA + (cc << 15)
00 mm cc  accA = accA + (($mm * cc) >> 7)
08 01 cc  $mm = accA; accA = accA + cc
08 02 cc  $mm = accA; accA = accA + (cc << 5)
08 03 cc  $mm = accA; accA = accA + (cc << 10)
08 04 cc  $mm = accA; accA = accA + (cc << 15)
08 mm cc  $mm = accA; accA = accA + (($mm * cc) >> 7)
10 01 cc  $mm = accB; accA = accA + cc
10 02 cc  $mm = accB; accA = accA + (cc << 5)
10 03 cc  $mm = accB; accA = accA + (cc << 10)
10 04 cc  $mm = accB; accA = accA + (cc << 15)
10 mm cc  $mm = accB; accA = accA + (($mm * cc) >> 7)
18 01 cc  $mm = accA_unsat; accA = accA + cc
18 02 cc  $mm = accA_unsat; accA = accA + (cc << 5)
18 03 cc  $mm = accA_unsat; accA = accA + (cc << 10)
18 04 cc  $mm = accA_unsat; accA = accA + (cc << 15)
18 mm cc  $mm = accA_unsat; accA = accA + (($mm * cc) >> 7)

20 01 cc  accA = cc
20 02 cc  accA = (cc << 5)
20 03 cc  accA = (cc << 10)
20 04 cc  accA = (cc << 15)
20 mm cc  accA = (($mm * cc) >> 7)
28 01 cc  $mm = accA; accA = cc
28 02 cc  $mm = accA; accA = (cc << 5)
28 03 cc  $mm = accA; accA = (cc << 10)
28 04 cc  $mm = accA; accA = (cc << 15)
28 mm cc  $mm = accA; accA = (($mm * cc) >> 7)
30 01 cc  $mm = accB; accA = cc
30 02 cc  $mm = accB; accA = (cc << 5)
30 03 cc  $mm = accB; accA = (cc << 10)
30 04 cc  $mm = accB; accA = (cc << 15)
30 mm cc  $mm = accB; accA = (($mm * cc) >> 7)
38 01 cc  $mm = accA_unsat; accA = cc
38 02 cc  $mm = accA_unsat; accA = (cc << 5)
38 03 cc  $mm = accA_unsat; accA = (cc << 10)
38 04 cc  $mm = accA_unsat; accA = (cc << 15)
38 mm cc  $mm = accA_unsat; accA = (($mm * cc) >> 7)


## Instructions 80-9f

1. store mem (as other instructions)
2. load mem / immediate for mul A
3. perform mul

coefs
    00  accA += 00
    01  accA += ($mem * (special $54 >> 16)) >> 7
    02  accA += ($mem * (special $55 >> 16)) >> 7
 u  03  accA += ($mem * (special $55 >> 16)) >> 7

 u  04  accA += (($mem * 0x40) >> 7) * -1
    05  accA += ($mem * (special $54 >> 16)) >> 7 * -1
 u  06  accA += ($mem * (special $55 >> 16)) >> 7 * -1
 u  07  accA += ($mem * (special $55 >> 16)) >> 7 * -1

 u  08  accA  = ($mem * 0x08) >> 7
    09  accA  = ($mem * (special $54 >> 16)) >> 7
    0a  accA  = ($mem * (special $55 >> 16)) >> 7
 u  0b  accA  = ($mem * (special $55 >> 16)) >> 7

 u  0c  accA  = (($mem * 0xc0) >> 7) * -1
    0d  accA  = ($mem * (special $54 >> 16)) >> 7 * -1
    0e  accA  = ($mem * (special $55 >> 16)) >> 7 * -1
 u  0f  accA  = ($mem * (special $55 >> 16)) >> 7 * -1
    1x  -> same but accB
    
  (4x/5x) commands only work after a (0x/1x)
 u  40
    41  acc += ($mem * ((special $54 & 0xffff) >> 15)) >> 1 >> 7
    42  acc += ($mem * ((special $55 & 0xffff) >> 15)) >> 1 >> 7
 u  43
 u  44
    45  acc += ($mem * ((special $54 & 0xffff) >> 15)) >> 1 >> 7 * -1
    46  acc += ($mem * ((special $55 & 0xffff) >> 15)) >> 1 >> 7 * -1
    5x  -> same but accB


## Instructions a0-b8

a0 01 cc  accA = abc(cc)
a0 02 cc  accA = abc(cc << 5)
a0 03 cc  accA = abc(cc << 10)
a0 04 cc  accA = abc(cc << 15)
a0 mm cc  accA = abc(($mm * cc) >> 7)
a8 01 cc  $mm = accA; accA = abc(cc)
a8 02 cc  $mm = accA; accA = abc(cc << 5)
a8 03 cc  $mm = accA; accA = abc(cc << 10)
a8 04 cc  $mm = accA; accA = abc(cc << 15)
a8 mm cc  $mm = accA; accA = abc(($mm * cc) >> 7)
b0 01 cc  $mm = accB; accA = abc(cc)
b0 02 cc  $mm = accB; accA = abc(cc << 5)
b0 03 cc  $mm = accB; accA = abc(cc << 10)
b0 04 cc  $mm = accB; accA = abc(cc << 15)
b0 mm cc  $mm = accB; accA = abc(($mm * cc) >> 7)
b8 01 cc  $mm = accA_unsat; accA = abc(cc)
b8 02 cc  $mm = accA_unsat; accA = abc(cc << 5)
b8 03 cc  $mm = accA_unsat; accA = abc(cc << 10)
b8 04 cc  $mm = accA_unsat; accA = abc(cc << 15)
b8 mm cc  $mm = accA_unsat; accA = abc(($mm * cc) >> 7)


## Instructions c0-ff

// TODO: rounding seems 1-off on cc
// uses the accumulator value from the prev instruction
c0 50 cc  accA = accA + ((accA * cc) >> 15)
c0 d0 cc  accA = accA + ((accA * cc) >> 13)
e0 50 cc  accB = accB + ((accB * cc) >> 15)
e0 d0 cc  accB = accB + ((accB * cc) >> 13)
c0 70 cc  accA = ((accA * cc) >> 15)
c0 f0 cc  accA = ((accA * cc) >> 13)
e0 70 cc  accB = ((accB * cc) >> 15)
e0 f0 cc  accB = ((accB * cc) >> 13)

// uses the acc value from the pipeline 3 before
c8 58 cc  $78 = accA; audio_out = accA; accA = accA + (accA * cc) >> 7
d0 58 cc  $78 = accB; audio_out = accB; accA = accA + (accB * cc) >> 7
d8 58 cc  $78 = accA; audio_out = accA_unsat; accA = accA + (accA * cc) >> 7  unsat??
c8 78 cc  $78 = accA; audio_out = accA; accA = (accA * cc) >> 7
d0 78 cc  $78 = accB; audio_out = accB; accA = (accB * cc) >> 7
d8 78 cc  $78 = accA; audio_out = accA_unsat; accA = (accA * cc) >> 7  unsat??

e8 58 cc  $78 = accA; audio_out = accA; accB = accB + (accA * cc) >> 7
f0 58 cc  $78 = accB; audio_out = accB; accB = accB + (accB * cc) >> 7
f8 58 cc  $78 = accA; audio_out = accA_unsat; accB = accB + (accA * cc) >> 7  unsat??
e8 78 cc  $78 = accA; audio_out = accA; accB = (accA * cc) >> 7
f0 78 cc  $78 = accB; audio_out = accB; accB = (accB * cc) >> 7
f8 78 cc  $78 = accA; audio_out = accA_unsat; accB = (accA * cc) >> 7  unsat??

// TODO: the audio in seems to be different with _old
c0 7e cc  $7e = special[0x7e]_old; accA = (special[0x7e] * cc) >> 7
c8 7e cc  $7e = special[0x7e]_old; accA = (special[0x7e] * cc) >> 7
e8 7e cc  $7e = special[0x7e]_old; accB = (special[0x7e] * cc) >> 7

c8 54 00  mult_a = accA
d0 54 00  mult_a = accB
d8 54 00  mult_a = accA_unsat


specials:
  4d  rep  ??
  4e  rep  ?? some timer/logic? used for square wave gen
  4f  rep  ?? some timer/logic? used for square wave gen
  50  inc  eram write latch
  53  inc  eram tap offs
  54  inc  mult a
  55  inc  mult b
  58  inc  audio out
  5a  inc  eram tap 1
  5b  inc  eram tap 2
  5c  inc  eram tap 3
  5d  inc  eram tap 4
  5e  rep  audio in
