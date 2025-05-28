from collections import defaultdict

def main():
  groups = defaultdict(list)
  with open("log", "r") as f:
    for line in f:
      line = line.strip()
      if not line:
        continue
      try:
        idx_str, val_str = line.split(": ")
        idx = int(idx_str, 16)
        val = int(val_str, 16)
        groups[idx].append(val)
      except ValueError:
        continue  # skip malformed lines

  for idx in sorted(groups):
    print(f"{idx:04x}: {' '.join(f'{v:06x}' for v in groups[idx])}")

if __name__ == "__main__":
  main()
