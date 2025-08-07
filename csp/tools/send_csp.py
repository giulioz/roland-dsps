import random
import time
import serial
import sys

import serial_lib

serial_lib.open_serial()


# pgm_f = open('upload.txt', 'r')
# pram_upload = []
# cram_upload = []
# for line in pgm_f:
#   _, datas = line.split(":")
#   data = datas.strip().split(" ")
#   pram_upload.append(0x00)
#   pram_upload.append(int(data[0], 16))
#   pram_upload.append(int(data[1], 16))
#   pram_upload.append(int(data[2], 16))
#   cram_upload.append(int(data[3], 16) >> 8)
#   cram_upload.append(int(data[3], 16) & 0xff)


serial_lib.clear_serial()

# serial_lib.clear_pram()
# serial_lib.clear_cram()

serial_lib.dsp_w(0x0804, 0x07) # 47, 07
serial_lib.dsp_w(0x0805, 0x80) # 00
serial_lib.dsp_w(0x0806, 0x02) # 00,01

serial_lib.dsp_w(0x0808, 0xe0) # e0  se70:0a,f6
serial_lib.dsp_w(0x0809, 0x00)
serial_lib.dsp_w(0x080a, 0x00)

# for i in range(0, len(pram_upload), 4):
#   print("writing pgm", hex(0x1000 + i))
#   serial_lib.dsp_w(0x1000 + i + 0, pram_upload[i + 3])
#   serial_lib.dsp_w(0x1000 + i + 1, pram_upload[i + 2])
#   serial_lib.dsp_w(0x1000 + i + 2, pram_upload[i + 1])
# for i in range(0, len(cram_upload), 2):
#   print("writing pgm", hex(i))
#   serial_lib.dsp_w(i + 0, cram_upload[i + 1])
#   serial_lib.dsp_w(i + 1, cram_upload[i + 0])

pos = 0
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1

serial_lib.dsp_pgm_w(pos, 0x00_20_01, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_0c_70, 0x0000); pos+=1

serial_lib.dsp_pgm_w(pos, 0x00_20_01, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x01_27_91, 0x4000); pos+=1

serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1

# serial_lib.dsp_pgm_w(pos, 0x00_50_70, 0xf000); pos+=1
# serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
# serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
# serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
# serial_lib.dsp_pgm_w(pos, 0x00_08_70, 0x0000); pos+=1

# serial_lib.dsp_pgm_w(pos, 0x01_20_03, 0x4000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
serial_lib.dsp_pgm_w(pos, 0x00_07_82, 0x0000); pos+=1


for i in range(40):
  serial_lib.dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1



# serial_lib.dsp_w(0x0804, 0x47)
# serial_lib.dsp_w(0x0805, 0x80)
# serial_lib.dsp_w(0x0806, 0x02)

# serial_lib.dsp_w(0x0808, 0x01)
# serial_lib.dsp_w(0x0809, 0x00)
# serial_lib.dsp_w(0x080a, 0x00)

# time.sleep(1)

# dump_cram()
# dump_pram()
# dump_cfg()

# for i in range(0x800, 0x1000, 4):
#   serial_lib.dsp_read_mem(i + 2)
#   f.write(f'{i:04x}: {serial_lib.dsp_r(0x00)[0]:02x} {serial_lib.dsp_r(0x01)[0]:02x} {serial_lib.dsp_r(0x02)[0]:02x} {serial_lib.dsp_r(0x03)[0]:02x}\n')

while True:
  serial_lib.dsp_read_mem(0x802)
  print(f'{serial_lib.dsp_r(0x02)[0]:02x}{serial_lib.dsp_r(0x01)[0]:02x}{serial_lib.dsp_r(0x00)[0]:02x}')
