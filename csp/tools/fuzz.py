import serial_lib
import emu_test
import random


serial_lib.open_serial()

serial_lib.clear_serial()

serial_lib.clear_pram()
serial_lib.clear_cram()

serial_lib.dsp_w(0x0804, 0x07)
serial_lib.dsp_w(0x0805, 0x80)
serial_lib.dsp_w(0x0806, 0x02)

serial_lib.dsp_w(0x0808, 0xe0)
serial_lib.dsp_w(0x0809, 0x00)
serial_lib.dsp_w(0x080a, 0x00)


pos = 0x00
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1

serial_lib.dsp_pgm_w(pos, 0x01_20_02, 0x0000); pos_setmem=pos; pos+=1  # memory val
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_08_70, 0x0000); pos+=1  # save unsat

serial_lib.dsp_pgm_w(pos, 0x01_20_02, 0x0000); pos_setm186=pos; pos+=1  # s186
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_05_86, 0x0000); pos+=1  # save unsat

serial_lib.dsp_pgm_w(pos, 0x01_20_02, 0x0000); pos_setm187=pos; pos+=1  # s187
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_05_87, 0x0000); pos_setm187_var=pos; pos+=1  # save unsat

serial_lib.dsp_pgm_w(pos, 0x00_20_01, 0x0000); pos_setaccA=pos; pos+=1  # accA
serial_lib.dsp_pgm_w(pos, 0x00_30_01, 0x0000); pos_setaccB=pos; pos+=1  # accB
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos_test=pos; pos+=1  # TEST
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_08_70, 0x0000); pos_sat=pos; pos+=1

serial_lib.dsp_pgm_w(pos, 0x01_20_70, 0x4000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_07_82, 0x0000); pos+=1  # log result accA

for i in range(48):
  serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1


while True:
  rnd_opcode = random.choice([
    0x0, 0x1, 0x2, 0x3,
    0x4,
    0x5,
    0x6, 0x7,
    0x8, 0x9,
    0xa, 0xb,
    0xc, 0xd, 0xe, 0xf
  ])
  rnd_mulshift = random.choice([0x0, 0x1, 0x2, 0x3])
  rnd_memslot = random.choice([0x70, 0x1, 0x2])
  rnd_coef = random.randint(0x0000, 0xffff)
  rnd_mulmode = random.choice([0x2, 0x3, 0x4, 0x5, 0x6, 0x7])
  
  rnd_mulshift_mem = random.choice([0x0, 0x1, 0x2, 0x3])
  rnd_immshift_mem = random.choice([0x1, 0x2])
  rnd_memval = random.randint(0x0000, 0xffff)
  
  rnd_mulshift_accA = random.choice([0x0, 0x1, 0x2, 0x3])
  rnd_immshift_accA = random.choice([0x1, 0x2])
  rnd_accA = random.randint(0x0000, 0xffff)
  
  rnd_mulshift_accB = random.choice([0x0, 0x1, 0x2, 0x3])
  rnd_immshift_accB = random.choice([0x1, 0x2])
  rnd_accB = random.randint(0x0000, 0xffff)

  rnd_mulshift_mul186 = random.choice([0x0, 0x1, 0x2, 0x3])
  rnd_immshift_mul186 = random.choice([0x1, 0x2])
  rnd_mul186 = random.randint(0x0000, 0xffff)
  
  rnd_mulshift_mul187 = random.choice([0x0, 0x1, 0x2, 0x3])
  rnd_immshift_mul187 = random.choice([0x1, 0x2])
  rnd_mul187 = random.randint(0x0000, 0xffff)

  rnd_dest_187 = random.choice([0x87, 0x85])
  
  rnd_saturate = random.choice([0x8, 0xc])


  # DEBUG
  rnd_opcode = 0xc
  rnd_mulshift = 0x0
  rnd_memslot = 0x70
  rnd_coef = 0x0000
  rnd_mulmode = 3
  
  rnd_memval = 0x0930
  rnd_mulshift_mem = 0x0
  rnd_immshift_mem = 0x1

  rnd_accA = 0x0000
  rnd_mulshift_accA = 0x0
  rnd_immshift_accA = 0x1
  
  rnd_accB = 0x0000
  rnd_mulshift_accB = 0x0
  rnd_immshift_accB = 0x1
  
  rnd_mul186 = 0x0000
  rnd_mulshift_mul186 = 0x0
  rnd_immshift_mul186 = 0x1
  
  rnd_mul187 = 0xf05f
  rnd_mulshift_mul187 = 0x0
  rnd_immshift_mul187 = 0x2

  rnd_dest_187 = 0x87
  
  rnd_saturate = 0xc

  rnd_opcode = 0x2
  rnd_mulshift = 0x0
  rnd_memslot = 0x70
  rnd_coef = 0x1000

  rnd_mulshift_mem = 0x2
  rnd_immshift_mem = 0x2
  rnd_memval = 0x4081



  serial_lib.dsp_pgm_w(pos_setmem, 0x00_20_00 | (rnd_mulshift_mem << 16) | rnd_immshift_mem, rnd_memval)  # memory val
  serial_lib.dsp_pgm_w(pos_setm186, 0x00_20_00 | (rnd_mulshift_mul186 << 16) | rnd_immshift_mul186, rnd_mul186)  # mul186
  serial_lib.dsp_pgm_w(pos_setm187, 0x00_20_00 | (rnd_mulshift_mul187 << 16) | rnd_immshift_mul187, rnd_mul187)  # mul187
  serial_lib.dsp_pgm_w(pos_setm187_var, 0x00_05_00 | rnd_dest_187, 0x0000)
  serial_lib.dsp_pgm_w(pos_setaccA, 0x00_20_00 | (rnd_mulshift_accA << 16) | rnd_immshift_accA, rnd_accA)  # accA
  serial_lib.dsp_pgm_w(pos_setaccB, 0x00_30_00 | (rnd_mulshift_accB << 16) | rnd_immshift_accB, rnd_accB)  # accB

  if rnd_opcode == 0xc or rnd_opcode == 0xd or rnd_opcode == 0xe or rnd_opcode == 0xf:
    serial_lib.dsp_pgm_w(pos_test, 0x00_00_00 | (rnd_mulshift << 16) | (rnd_opcode << 12) | rnd_memslot, rnd_mulmode)
    # print(hex(0x00_00_00 | (rnd_mulshift << 16) | (rnd_opcode << 12) | rnd_memslot), hex(rnd_mulmode))
  else:
    serial_lib.dsp_pgm_w(pos_test, 0x00_00_00 | (rnd_mulshift << 16) | (rnd_opcode << 12) | rnd_memslot, rnd_coef)
    # print(hex(0x00_00_00 | (rnd_mulshift << 16) | (rnd_opcode << 12) | rnd_memslot), hex(rnd_coef))
  
  serial_lib.dsp_pgm_w(pos_sat, 0x00_00_70 | (rnd_saturate << 8), 0x0000)  # log result accA


  in_accA = emu_test.sign_extend(rnd_accA, 16)
  if rnd_immshift_accA == 0x1:
    in_accA = (in_accA << 0)
  elif rnd_immshift_accA == 0x2:
    in_accA = (in_accA << 7)
  if rnd_mulshift_accA == 0x0:
    in_accA = (in_accA << 0)
  elif rnd_mulshift_accA == 0x1:
    in_accA = (in_accA << 1)
  elif rnd_mulshift_accA == 0x2:
    in_accA = (in_accA << 2)
  elif rnd_mulshift_accA == 0x3:
    in_accA = (in_accA << 4)
  in_accA = in_accA & (2**emu_test.acc_size - 1)

  in_accB = emu_test.sign_extend(rnd_accB, 16)
  if rnd_immshift_accB == 0x1:
    in_accB = (in_accB << 0)
  elif rnd_immshift_accB == 0x2:
    in_accB = (in_accB << 7)
  if rnd_mulshift_accB == 0x0:
    in_accB = (in_accB << 0)
  elif rnd_mulshift_accB == 0x1:
    in_accB = (in_accB << 1)
  elif rnd_mulshift_accB == 0x2:
    in_accB = (in_accB << 2)
  elif rnd_mulshift_accB == 0x3:
    in_accB = (in_accB << 4)
  in_accB = in_accB & (2**emu_test.acc_size - 1)

  in_mul186 = emu_test.sign_extend(rnd_mul186, 16)
  if rnd_immshift_mul186 == 0x1:
    in_mul186 = (in_mul186 << 0)
  elif rnd_immshift_mul186 == 0x2:
    in_mul186 = (in_mul186 << 7)
  if rnd_mulshift_mul186 == 0x0:
    in_mul186 = (in_mul186 << 0)
  elif rnd_mulshift_mul186 == 0x1:
    in_mul186 = (in_mul186 << 1)
  elif rnd_mulshift_mul186 == 0x2:
    in_mul186 = (in_mul186 << 2)
  elif rnd_mulshift_mul186 == 0x3:
    in_mul186 = (in_mul186 << 4)
  in_mul186 = in_mul186 & (2**24 - 1)

  in_mul187 = emu_test.sign_extend(rnd_mul187, 16)
  if rnd_immshift_mul187 == 0x1:
    in_mul187 = (in_mul187 << 0)
  elif rnd_immshift_mul187 == 0x2:
    in_mul187 = (in_mul187 << 7)
  if rnd_mulshift_mul187 == 0x0:
    in_mul187 = (in_mul187 << 0)
  elif rnd_mulshift_mul187 == 0x1:
    in_mul187 = (in_mul187 << 1)
  elif rnd_mulshift_mul187 == 0x2:
    in_mul187 = (in_mul187 << 2)
  elif rnd_mulshift_mul187 == 0x3:
    in_mul187 = (in_mul187 << 4)
  in_mul187 = in_mul187 & (2**24 - 1)

  if rnd_dest_187 == 0x85:
    in_mul186 = (in_mul187 & 0x3ff) << 13

  in_mem = emu_test.sign_extend(rnd_memval, 16)
  if rnd_immshift_mem == 0x1:
    in_mem = (in_mem << 0)
  elif rnd_immshift_mem == 0x2:
    in_mem = (in_mem << 7)
  if rnd_mulshift_mem == 0x0:
    in_mem = (in_mem << 0)
  elif rnd_mulshift_mem == 0x1:
    in_mem = (in_mem << 1)
  elif rnd_mulshift_mem == 0x2:
    in_mem = (in_mem << 2)
  elif rnd_mulshift_mem == 0x3:
    in_mem = (in_mem << 4)
  in_mem = in_mem & (2**24 - 1)

  if rnd_opcode == 0xc or rnd_opcode == 0xd or rnd_opcode == 0xe or rnd_opcode == 0xf:
    emu = emu_test.step(rnd_opcode, rnd_mulshift, rnd_memslot, rnd_mulmode, in_accA, in_accB, in_mem, in_mul186, in_mul187)
  else:
    emu = emu_test.step(rnd_opcode, rnd_mulshift, rnd_memslot, rnd_coef, in_accA, in_accB, in_mem, in_mul186, in_mul187)
  emu_accA = emu[0]
  
  if rnd_saturate == 0xc:
    emu_accA = emu_test.saturate_24(emu_accA)
  emu_accA &= 0xffffff

  serial_lib.dsp_read_mem(0x802)
  result = (serial_lib.dsp_r(0x02)[0] << 16) | (serial_lib.dsp_r(0x01)[0] << 8) | serial_lib.dsp_r(0x00)[0]
  print(f'opcode:{rnd_opcode:x} mem:{(in_mem):06x},  accA:{(in_accA):06x},  accB:{(in_accB):06x},  m186:{(in_mul186):06x},  m187:{(in_mul187):06x},  sat:{rnd_saturate:01x},  mulmode:{rnd_mulmode:01x}')
  # print(f'cmd tst:{(0x00_00_00 | (rnd_mulshift << 16) | (rnd_opcode << 12) | rnd_memslot):06x} {rnd_coef:04x}')
  print(f'result: {result:06x}   emu: {emu_accA:06x}  {'OK' if result == emu_accA else ('ERROR ' + str(result - emu_accA))}\n')
  # if result != emu_accA:
  #   # print(f'ERROR: {result:06x} != {emu_accA:06x}  off by {result - emu_accA:06x}')
  #   break
