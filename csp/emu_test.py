def sign_extend(x, bit_count):
    if x & (1 << (bit_count - 1)):
        return x - (1 << bit_count)
    return x

mem_pos = 0
mem = [0] * 0x200

accA = 0
accB = 0

def step(opcode, shift_bits, mem_offs, coef_unsigned):
  coef_signed = sign_extend(coef_unsigned, 16)

  mem_val = mem[(mem_offs + mem_pos) & 0x1ff] * coef_signed
  if mem_offs == 1:
    mem_val = coef_signed << 15
  elif mem_offs == 2:
    mem_val = coef_signed << 32

  mem_val &= 0x3fffffffff

  shift = 0
  if shift_bits == 0:
    shift = 15
  elif shift_bits == 1:
    shift = 14
  elif shift_bits == 2:
    shift = 13
  elif shift_bits == 3:
    shift = 11


  # MAC
  if opcode == 0:
    accA += mem_val >> shift
  elif opcode == 1:
    accB += mem_val >> shift
  elif opcode == 2:
    accA = mem_val >> shift
  elif opcode == 3:
    accB = mem_val >> shift
  
  # ABS_CLAMP
  elif opcode == 4 or opcode == 5:
    if mem_val < 0:
      mem_val = ~mem_val
    mem_val &= 0x3fffffffff
    # sign extend?
    mem_val >>= shift
    
    if opcode == 4:
      accA = mem_val
    elif opcode == 5:
      accA += mem_val
    if accA < 0:
      accA = 0

  # NEG
  elif opcode == 6 or opcode == 7:
    if mem_val >= 0:
      mem_val = ~mem_val
    mem_val &= 0x3fffffffff
    # sign extend?
    mem_val >>= shift
    
    if opcode == 6:
      accA = mem_val
    elif opcode == 7:
      accA += mem_val
  
  # ABS
  elif opcode == 8 or opcode == 9:
    if mem_val < 0:
      mem_val = ~mem_val
    mem_val &= 0x3fffffffff
    # sign extend?
    mem_val >>= shift
    
    if opcode == 8:
      accA = mem_val
    elif opcode == 9:
      accA += mem_val

  # MAC_LOW
  elif opcode == 0xa:
    accA += mem_val >> 15 >> shift
  elif opcode == 0xb:
    accB += mem_val >> 15 >> shift
