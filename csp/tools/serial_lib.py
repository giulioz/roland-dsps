import time
import serial
import sys

def open_serial():
  global ser
  if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} <serial_port>")
    sys.exit(1)
  serial_port = sys.argv[1]
  baudrate = 921600

  ser = serial.Serial(serial_port, baudrate, timeout=10000)
  time.sleep(1)  

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
    print(f'{i:04x}: {dsp_r(0x00)[0]:02x} {dsp_r(0x01)[0]:02x} {dsp_r(0x02)[0]:02x}')

def dump_cram():
  for i in range(0x0000, 0x0800, 2):
    print("reading", hex(i))
    # dsp_wait()
    dsp_read_mem(i + 1)
    # dsp_wait()
    print(f'{i:04x}: {dsp_r(0x00)[0]:02x} {dsp_r(0x01)[0]:02x}')

def dump_cfg():
  for i in range(0x0800, 0x080f, 1):
    print("reading", hex(i))
    # dsp_wait()
    dsp_read_mem(i)
    # dsp_wait()
    print(f'{dsp_r(0x00)[0]:02x} {dsp_r(0x01)[0]:02x} {dsp_r(0x02)[0]:02x} {dsp_r(0x03)[0]:02x}')

def dsp_pgm_w(pos, instr, coef):
  dsp_w(0x1000 + pos * 4 + 0, instr & 0xff)
  dsp_w(0x1000 + pos * 4 + 1, (instr >> 8) & 0xff)
  dsp_w(0x1000 + pos * 4 + 2, (instr >> 16) & 0xff)

  dsp_w(0x0000 + pos * 2 + 0, coef & 0xff)
  dsp_w(0x0000 + pos * 2 + 1, (coef >> 8) & 0xff)

  # print(f'pgm_w {pos:04x}: {instr:06x} {coef:04x}')
