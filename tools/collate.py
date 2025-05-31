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


  # ALIGNMENT WITH MAGIC 000001
  # Sort groups by key (already done in the print loop)
  # Rotate and wrap values until the value in key 0 is 1
  while groups and (not groups[0] or groups[0][0] != 1):
    # Collect the first value from each group in key order
    keys = sorted(groups)
    first_vals = []
    for k in keys:
      if groups[k]:
        first_vals.append(groups[k].pop(0))
      else:
        first_vals.append(None)
    # Rotate left
    first_vals = first_vals[1:] + first_vals[:1]
    # Put back the rotated values
    for i, k in enumerate(keys):
      if first_vals[i] is not None:
        groups[k].insert(0, first_vals[i])


  for idx in sorted(groups):
    print(f"{idx:04x}: {' '.join(f'{v:06x}' for v in groups[idx])}")
  # print(f"{' '.join(f'{v:06x}' for v in groups[1])}")
  # print(f"{' '.join(f'{v:06x}' for v in groups[2])}")

if __name__ == "__main__":
  main()
