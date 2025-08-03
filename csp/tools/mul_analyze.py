from emu_test import sign_extend


def read_mul_data(filename):
  results = []
  with open(filename, 'r') as f:
    for line in f:
      line = line.strip()
      if not line:
        continue
      parts = line.split()
      entry = {}
      for part in parts:
        key, value = part.split(':')
        entry[key] = int(value, 16)
      results.append(entry)
  return results

data = read_mul_data('mul_data')

# for entry in data[0:1]:
ok = 0
max_delta = 0
for entry in data:
  mem = entry['mem']
  m = entry['m187']
  result = entry['result']

  m = sign_extend(m, 24)
  mem = sign_extend(mem, 24)
  result = sign_extend(result, 24)

  # print(f'mem: {mem:06x}, m187: {m:06x}, result: {result:06x}')


  mul = (~mem * (~(m & 0xffffff) >> 8))
  mul_s = ~mul >> 15
  emu = mem + mul_s

  delta = abs(emu - result)
  if delta > max_delta:
    max_delta = delta

  if emu == result:
    print(f"OK  : mem: {(mem&0xffffff):06x}, m187: {(m&0xffffff):06x}, result: {(result&0xffffff):06x}, emu: {(emu&0xffffff):06x}")
    ok += 1
  else:
    print(f"FAIL: mem: {(mem&0xffffff):06x}, m187: {(m&0xffffff):06x}, result: {(result&0xffffff):06x}, emu: {(emu&0xffffff):06x}, delta: {delta:06x}")

print(f"OK: {ok} / {len(data)}")
print(f"Max delta: {max_delta:06x}")
