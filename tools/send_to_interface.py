import os
import time
import serial
import sys

def read_hex_file(filename):
  with open(filename, 'r') as f:
    for line in f:
      if line.startswith('//'):
          continue
      if line.strip() == '':
          continue
      parts = line.strip().split()[0:3]
      parts.append('00')
      # print(list(hex(int(b, 16)) for b in parts))
      try:
        yield bytes(int(b, 16) for b in parts)
      except ValueError:
        continue  # skip lines with invalid hex

def send_to_serial(port, baudrate, data_iter):
  with serial.Serial(port, baudrate, timeout=0.1) as ser:
    time.sleep(2)
    for data in data_iter:
      # print(data)
      ser.write(data)
    # output = ser.readlines()
    # for line in output:
    #   print(line.decode('utf-8').strip())
    while True:
    # while ser.in_waiting > 0:
        line = ser.readline().decode('utf-8', errors='replace').strip()
        if line:
          print(line)
          if line.startswith('7F:'):
            break

if __name__ == "__main__":
  if len(sys.argv) != 3:
    print(f"Usage: {sys.argv[0]} <hex_file.txt> <serial_port>")
    sys.exit(1)
  hex_file = sys.argv[1]
  serial_port = sys.argv[2]
  baudrate = 115200

  data_iter = read_hex_file(hex_file)
  send_to_serial(serial_port, baudrate, data_iter)
