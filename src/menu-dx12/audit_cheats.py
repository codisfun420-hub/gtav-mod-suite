import re
import json

with open('natives.json') as f:
    natives = json.load(f)

hash_map = {}
for ns, n_dict in natives.items():
    for h, data in n_dict.items():
        hash_map[h.upper()] = (ns, data.get('name', 'UNKNOWN'), data.get('params', []), data.get('return_type', 'void'))

with open('src/Cheats.cpp') as f:
    lines = f.readlines()

for i, line in enumerate(lines):
    hashes = re.findall(r'0x[0-9A-Fa-f]{16}', line)
    for h in hashes:
        h_norm = '0X' + h[2:].upper()
        if h_norm in hash_map:
            ns, name, params, ret = hash_map[h_norm]
            p_str = ', '.join(f"{p.get('type')} {p.get('name')}" for p in params)
            print(f"Line {i+1}: {h} -> {ns}::{name}({p_str}) -> {ret}")
        else:
            print(f"Line {i+1}: {h} -> NOT FOUND IN NATIVES.JSON! Line: {line.strip()}")
