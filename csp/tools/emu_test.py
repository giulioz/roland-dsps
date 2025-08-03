def sign_extend(x, bit_count):
    if x & (1 << (bit_count - 1)):
        return x - (1 << bit_count)
    return x

def saturate_24(x):
    if x < -8388608:
        return -8388608
    elif x > 8388607:
        return 8388607
    return x

# mem_pos = 0
# mem = [0] * 0x200

acc_size = 38
acc_mask = (1 << acc_size) - 1

def step(opcode, shift_bits, mem_offs, coef_unsigned, accA, accB, mem_val, mul186, mul187):
  coef_signed = sign_extend(coef_unsigned, 16)

  accA = sign_extend(accA, 38)
  accB = sign_extend(accB, 38)
  mem_val = sign_extend(mem_val, 24)

  # mem_val = mem[(mem_offs + mem_pos) & 0x1ff] * coef_signed
  mem_val_orig = mem_val
  mem_val = mem_val * coef_signed
  if mem_offs == 1:
    mem_val = coef_signed << 15
  elif mem_offs == 2:
    mem_val = coef_signed << 22

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
    mem_val &= acc_mask
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
    mem_val &= acc_mask
    mem_val >>= shift
    
    if opcode == 6:
      accA = mem_val
    elif opcode == 7:
      accA += mem_val
  
  # ABS
  elif opcode == 8 or opcode == 9:
    if mem_val < 0:
      mem_val = ~mem_val
    mem_val &= acc_mask
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

  elif opcode == 0xc or opcode == 0xd or opcode == 0xe or opcode == 0xf:
    if mem_offs == 1:
      mem_val_orig = 0x8000
    elif mem_offs == 2:
      mem_val_orig = 0x8000 << 7

    m = sign_extend(mul186, 24)
    if coef_signed & 1 != 0:
      m = sign_extend(mul187, 24)

    mul_val = 0
    if coef_signed == 0x2 or coef_signed == 0x3:
      mul = (~mem_val_orig * (~(m & 0xffffff) >> 8))
      mul_s = mul >> 15
      mul_val = mem_val_orig - mul_s - 1
    elif coef_signed == 0x4 or coef_signed == 0x5:
      mul_val = (mem_val_orig * ~(m >> 8)) >> shift
    elif coef_signed == 0x6 or coef_signed == 0x7:
      mul_val = (mem_val_orig * (m >> 8)) >> shift

    if opcode == 0xc:
      accA = mul_val
    elif opcode == 0xd:
      accB = mul_val
    elif opcode == 0xe:
      accA += mul_val
    elif opcode == 0xf:
      accB += mul_val

  return [accA, accB]
