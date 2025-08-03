import random
import time
import serial
import sys

if len(sys.argv) != 2:
  print(f"Usage: {sys.argv[0]} <serial_port>")
  sys.exit(1)
serial_port = sys.argv[1]
baudrate = 921600

ser = serial.Serial(serial_port, baudrate, timeout=10000)
time.sleep(2)

def clear_serial():
  while ser.in_waiting > 0:
    ser.read()



def dsp_w(addr, data):
  ser.write(bytes([b'w'[0], (addr >> 8) & 0xff, addr & 0xff, data]))
  ser.read()

def dsp_r(addr):
  ser.write(bytes([b'r'[0], (addr >> 8) & 0xff, addr & 0xff, 0x00]))
  return ser.read()

def dsp_read_mem(addr):
  dsp_w(addr + 0x2000, 0x00)

def dsp_wait(i = 3):
  while True:
    if int(dsp_r(i)[0]) == 0x01:
      break

f = open('log', 'w')
def dsp_read_mem_log(addr):
  print("reading", hex(addr))
  dsp_read_mem(addr)
  dsp_r(0x00)
  f.write(f'{dsp_r(0x01)[0]:02x} {dsp_r(0x02)[0]:02x} {dsp_r(0x03)[0]:02x} {dsp_r(0x00)[0]:02x}\n')
  dsp_r(0x00)

def clear_pram():
  for i in range(0x1000, 0x2000, 4):
    print("clearing", hex(i))
    dsp_w(i + 0, 0x00)
    dsp_w(i + 1, 0x00)
    dsp_w(i + 2, 0x00)
    dsp_w(i + 3, 0x00)
  
def clear_cram():
  for i in range(0x0000, 0x800, 2):
    print("clearing", hex(i))
    dsp_w(i, 0x00)
    dsp_w(i + 1, 0x00)

def dump_pram():
  for i in range(0x1000, 0x2000, 4):
    print("reading", hex(i))
    # dsp_wait()
    dsp_read_mem(i + 2)
    # dsp_wait()
    f.write(f'{i:04x}: {dsp_r(0x00)[0]:02x} {dsp_r(0x01)[0]:02x} {dsp_r(0x02)[0]:02x}\n')

def dump_cram():
  for i in range(0x0000, 0x0800, 2):
    print("reading", hex(i))
    # dsp_wait()
    dsp_read_mem(i + 1)
    # dsp_wait()
    f.write(f'{i:04x}: {dsp_r(0x00)[0]:02x} {dsp_r(0x01)[0]:02x}\n')

def dump_cfg():
  for i in range(0x0800, 0x080f, 1):
    print("reading", hex(i))
    # dsp_wait()
    dsp_read_mem(i)
    # dsp_wait()
    f.write(f'{dsp_r(0x00)[0]:02x} {dsp_r(0x01)[0]:02x} {dsp_r(0x02)[0]:02x} {dsp_r(0x03)[0]:02x}\n')

def dsp_pgm_w(pos, instr, coef):
  dsp_w(0x1000 + pos * 4 + 0, instr & 0xff)
  dsp_w(0x1000 + pos * 4 + 1, (instr >> 8) & 0xff)
  dsp_w(0x1000 + pos * 4 + 2, (instr >> 16) & 0xff)

  dsp_w(0x0000 + pos * 2 + 0, coef & 0xff)
  dsp_w(0x0000 + pos * 2 + 1, (coef >> 8) & 0xff)



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


clear_serial()

# clear_pram()
# clear_cram()

dsp_w(0x0804, 0x07) # 47, 07
dsp_w(0x0805, 0x80) # 00
dsp_w(0x0806, 0x02) # 00,01

dsp_w(0x0808, 0xe0) # e0  se70:0a,f6
dsp_w(0x0809, 0x00)
dsp_w(0x080a, 0x00)

# for i in range(0, len(pram_upload), 4):
#   print("writing pgm", hex(0x1000 + i))
#   dsp_w(0x1000 + i + 0, pram_upload[i + 3])
#   dsp_w(0x1000 + i + 1, pram_upload[i + 2])
#   dsp_w(0x1000 + i + 2, pram_upload[i + 1])
# for i in range(0, len(cram_upload), 2):
#   print("writing pgm", hex(i))
#   dsp_w(i + 0, cram_upload[i + 1])
#   dsp_w(i + 1, cram_upload[i + 0])

pos = 0
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x88_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1


# clear regs and temp
dsp_pgm_w(pos, 0x00_30_01, 0x0000); pos+=1 #
dsp_pgm_w(pos, 0x00_20_01, 0x0000); pos+=1 #
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_07_8a, 0x0000); pos+=1 #


# save test value to mem
dsp_pgm_w(pos, 0x08_20_01, 0x0000); pos+=1 #/ 
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_0c_70, 0x0000); pos+=1 #
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1


# ram offs
dsp_pgm_w(pos, 0x00_20_01, 0x0400); pos+=1 #
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_07_85, 0x0000); pos+=1 #


# test
dsp_pgm_w(pos, 0x00_20_01, 0x0000); pos+=1 #
dsp_pgm_w(pos, 0x00_30_01, 0x0000); pos+=1 ##
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_c0_70, 0x0002); pos+=1 #
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_08_70, 0x0000); pos+=1 # unsat
# dsp_pgm_w(pos, 0x00_0c_70, 0x0000); pos+=1 # sat



# output temp from mem
dsp_pgm_w(pos, 0x01_20_70, 0x4000); pos+=1 #
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_07_82, 0x0000); pos+=1 #


dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1
dsp_pgm_w(pos, 0x00_00_00, 0x0000); pos+=1


# dsp_w(0x0804, 0x47)
# dsp_w(0x0805, 0x80)
# dsp_w(0x0806, 0x02)

# dsp_w(0x0808, 0x01)
# dsp_w(0x0809, 0x00)
# dsp_w(0x080a, 0x00)

# time.sleep(1)

# dump_cram()
# dump_pram()
# dump_cfg()

# for i in range(0x800, 0x1000, 4):
#   dsp_read_mem(i + 2)
#   f.write(f'{i:04x}: {dsp_r(0x00)[0]:02x} {dsp_r(0x01)[0]:02x} {dsp_r(0x02)[0]:02x} {dsp_r(0x03)[0]:02x}\n')

# while True:
#   dsp_read_mem(0x802)
#   print(f'{dsp_r(0x02)[0]:02x}{dsp_r(0x01)[0]:02x}{dsp_r(0x00)[0]:02x}')
