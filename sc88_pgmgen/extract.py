rom = open('sc88pro_valid.bin', 'rb')

# rom.seek(0xfe000)
# names_data = rom.read(0x34c)
# names_strings = [names_data[i*0xd:(i+1)*0xd].decode('ascii', errors='ignore').rstrip('\0')
#        for i in range(0, 0x34c // 0xd)]
# for i, name in enumerate(names_strings):
#     if name:
#         print(f"{i:03d}: {name}")

# print("\n")

# rom.seek(0xdd624)
# params_data = rom.read(0xa71)
# params_strings = [params_data[i*0xb:(i+1)*0xb].decode('ascii', errors='ignore').rstrip('\0')
#        for i in range(0, 0xa71 // 0xb)]
# for i, name in enumerate(params_strings):
#     if name:
#         print(f"{i:03d}: {name}")

for i in range(0x3b31 // 0x10):
  rom.seek(0xd92f6 + i * 0x10 + 0x8)
  ptr = rom.read(2)
  rom.seek(0xd0000 | int.from_bytes(ptr, 'big'))
  string = rom.read(0x10).decode('ascii', errors='ignore').rstrip('\0')
  print(f"{string}")

# around 0xe7ba0 params ptr table (0x2 each)
# around 0xe7be2 params table (0x6 each)
  # 0x3 lsp dest ptr


rom.close()
