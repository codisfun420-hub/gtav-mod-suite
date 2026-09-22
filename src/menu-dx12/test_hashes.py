import json
import re

with open("natives.json", "r", encoding="utf-8") as f:
    natives = json.load(f)

# Flatten
canonical = {}
for ns, n_dict in natives.items():
    for h, data in n_dict.items():
        name = data.get("name", "")
        if name:
            canonical[name] = (h.upper(), ns)

required_mappings = [
    ("IS_CONTROL_PRESSED", "0xF3A21BCD95725A4A"),
    ("IS_DISABLED_CONTROL_PRESSED", "0xE2587F8CBBD87B1D"),
    ("IS_CONTROL_JUST_PRESSED", "0x580417101DDB492F"),
    ("IS_DISABLED_CONTROL_JUST_PRESSED", "0x91AEF906BCA88877"),
    ("GET_DISABLED_CONTROL_NORMAL", "0x11E65974A982637C"),
    ("RESET_PLAYER_STAMINA", "0xA6F312FCCE9C1DFE"),
    ("SET_VEHICLE_REDUCE_GRIP", "0x222FF6A823D122E2"),
    ("GET_ENTITY_ROLL", "0x831E0242595560DF"),
    ("SET_VEHICLE_ON_GROUND_PROPERLY", "0x49733E92263139D1"),
    ("SET_VEHICLE_CUSTOM_PRIMARY_COLOUR", "0x7141766F91D15BEA"),
    ("SET_VEHICLE_CUSTOM_SECONDARY_COLOUR", "0x36CED73BFED89754"),
    ("SET_EXPLOSIVE_MELEE_THIS_FRAME", "0xFF1BED81BFDC0FE0"),
    ("SET_TIME_SCALE", "0x1D408577D440E81E"),
    ("SET_ARTIFICIAL_LIGHTS_STATE", "0x1268615ACE24D504"),
    ("RESET_PED_VISIBLE_DAMAGE", "0x3AC1F7B898F30C05"),
    ("SPECIAL_ABILITY_FILL_METER", "0x3DACA8DDC6FD4980"),
    ("SET_PED_TO_RAGDOLL", "0xAE99FB955581844A"),
    ("SET_VEHICLE_MOD", "0x6AF0636DDEDCB6DD"),
    ("TOGGLE_VEHICLE_MOD", "0x2A1F4F37F95BAD08"),
    ("SET_VEHICLE_TYRES_CAN_BURST", "0xEB9DC3C7D8596C46"),
    ("SET_VEHICLE_NUMBER_PLATE_TEXT", "0x95A88F0B409CDA47"),
    ("GET_NUM_VEHICLE_MODS", "0xE38E9162A2500646"),
    ("GET_VEHICLE_MOD", "0x772960298DA26FDB"),
    ("SET_VEHICLE_NEON_ENABLED", "0x2AA720E4287BF269"),
    ("SET_VEHICLE_NEON_COLOUR", "0x8E0A582209A62695"),
    ("SET_VEHICLE_XENON_LIGHT_COLOR_INDEX", "0xE41033B25D003A07"),
    ("SET_VEHICLE_WINDOW_TINT", "0x57C51E6BAD752696"),
    ("SET_VEHICLE_WHEEL_TYPE", "0x487EB21CC7295BA1"),
    ("SET_VEHICLE_TYRE_SMOKE_COLOR", "0xB5BA80F839791C0F"),
    ("FREEZE_ENTITY_POSITION", "0x428CA6DBD1094446"),
    ("PLAY_SOUND_FRONTEND", "0x67C540AA08E4A6F5"),
]

print("Checking canonical hashes in natives.json...")
for name, expected_hash in required_mappings:
    if name in canonical:
        actual_h, ns = canonical[name]
        assert actual_h == expected_hash.upper(), f"Mismatch for {name}: expected {expected_hash}, got {actual_h}"
        print(f"  [OK] {name} ({ns}) -> {actual_h}")
    else:
        print(f"  [MISSING NAME] {name}")

# Now check Cheats.cpp
with open("src/Cheats.cpp", "r", encoding="utf-8") as f:
    cheats_cpp = f.read()

print("\nChecking presence of canonical hashes in Cheats.cpp...")
for name, expected_hash in required_mappings:
    count = len(re.findall(expected_hash, cheats_cpp, re.IGNORECASE))
    assert count > 0, f"Hash {expected_hash} ({name}) not found in Cheats.cpp!"
    print(f"  [OK] {expected_hash} ({name}) found {count} time(s)")

print("\nALL 31 CANONICAL NATIVE HASHES VERIFIED IN NATIVES.JSON AND CHEATS.CPP!")
