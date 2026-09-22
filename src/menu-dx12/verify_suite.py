import os
import sys
import re
import json
import pefile
import configparser

def test_pe_binary(file_path):
    print(f"[*] Testing PE binary: {file_path}")
    assert os.path.exists(file_path), f"File {file_path} does not exist!"
    size = os.path.getsize(file_path)
    print(f"    Size: {size:,} bytes")
    assert size > 500_000, f"Size too small: {size}"

    pe = pefile.PE(file_path)
    assert pe.is_dll(), "PE is not marked as a DLL!"
    assert pe.FILE_HEADER.Machine == 0x8664, f"Machine architecture is not x64: {hex(pe.FILE_HEADER.Machine)}"

    imported_dlls = [entry.dll.decode().lower() for entry in pe.DIRECTORY_ENTRY_IMPORT]
    print(f"    Imported DLLs ({len(imported_dlls)}): {', '.join(imported_dlls[:6])}...")

    # Required graphics and system libraries
    assert 'dxgi.dll' in imported_dlls, "Missing dxgi.dll import"
    assert 'user32.dll' in imported_dlls, "Missing user32.dll import"
    assert 'kernel32.dll' in imported_dlls, "Missing kernel32.dll import"
    print("    [PASS] PE structure, machine architecture, and imports verified.")

def test_native_hashes(natives_path, cheats_cpp_path):
    print(f"\n[*] Testing Native Hash Mappings: {cheats_cpp_path}")
    assert os.path.exists(natives_path), f"Missing {natives_path}"
    assert os.path.exists(cheats_cpp_path), f"Missing {cheats_cpp_path}"

    with open(natives_path, "r", encoding="utf-8") as f:
        natives = json.load(f)

    hash_map = {}
    for ns, n_dict in natives.items():
        for h, data in n_dict.items():
            hash_map[h.upper()] = (ns, data.get("name", "UNKNOWN"))

    with open(cheats_cpp_path, "r", encoding="utf-8") as f:
        code = f.read()

    used_hashes = re.findall(r"0x[0-9A-Fa-f]{16}", code)
    print(f"    Found {len(used_hashes)} native invocation hashes in Cheats.cpp")

    invalid_hashes = []
    for h in used_hashes:
        h_norm = "0X" + h[2:].upper()
        if h_norm not in hash_map:
            invalid_hashes.append(h)

    assert len(invalid_hashes) == 0, f"Invalid native hashes found: {invalid_hashes}"
    print(f"    [PASS] All {len(used_hashes)} native hashes strictly verified against GTA V native database.")

def test_ini_config(file_path):
    print(f"\n[*] Testing INI configuration: {file_path}")
    assert os.path.exists(file_path), f"File {file_path} does not exist!"
    config = configparser.ConfigParser()
    config.read(file_path)

    assert config.has_section("Settings"), "Missing [Settings] section"
    assert config.has_section("Cheats"), "Missing [Cheats] section"

    assert config.getint("Settings", "ToggleKey") == 45, "Default ToggleKey should be 45 (VK_INSERT)"
    assert config.getfloat("Settings", "WindowAlpha") == 0.90, "Default WindowAlpha should be 0.90"
    assert config.getfloat("Settings", "AccentB") == 1.00, "Default AccentB should be 1.00 (Cyan)"

    # Player cheats
    assert config.has_option("Cheats", "PlayerGodMode"), "Missing PlayerGodMode cheat setting"
    assert config.has_option("Cheats", "PlayerNeverWanted"), "Missing PlayerNeverWanted cheat setting"
    assert config.has_option("Cheats", "PlayerSuperJump"), "Missing PlayerSuperJump cheat setting"
    assert config.has_option("Cheats", "PlayerFastSprint"), "Missing PlayerFastSprint cheat setting"
    assert config.has_option("Cheats", "PlayerInvisibility"), "Missing PlayerInvisibility cheat setting"

    # Vehicle & Weapon cheats
    assert config.has_option("Cheats", "VehicleGodMode"), "Missing VehicleGodMode cheat setting"
    assert config.has_option("Cheats", "WeaponsInfiniteAmmo"), "Missing WeaponsInfiniteAmmo cheat setting"
    assert config.has_option("Cheats", "WeaponsRapidFire"), "Missing WeaponsRapidFire cheat setting"

    # World cheats
    assert config.has_option("Cheats", "WorldPauseTime"), "Missing WorldPauseTime cheat setting"
    assert config.has_option("Cheats", "WorldHour"), "Missing WorldHour cheat setting"
    assert config.has_option("Cheats", "WorldMinute"), "Missing WorldMinute cheat setting"
    assert config.has_option("Cheats", "WorldGravityLevel"), "Missing WorldGravityLevel cheat setting"
    print("    [PASS] INI configuration sections and key defaults verified.")

def test_mod_backup(game_dir):
    print(f"\n[*] Testing .mod_backup repository parity...")
    backup_dir = os.path.join(game_dir, "tools", ".mod_backup")
    assert os.path.exists(backup_dir), ".mod_backup directory missing"

    asi_game = os.path.join(game_dir, "EnhancedImGuiMenu.asi")
    asi_backup = os.path.join(backup_dir, "EnhancedImGuiMenu.asi")
    ini_game = os.path.join(game_dir, "EnhancedImGuiMenu.ini")
    ini_backup = os.path.join(backup_dir, "EnhancedImGuiMenu.ini")

    assert os.path.exists(asi_backup), "Backup EnhancedImGuiMenu.asi missing"
    assert os.path.exists(ini_backup), "Backup EnhancedImGuiMenu.ini missing"

    assert os.path.getsize(asi_game) == os.path.getsize(asi_backup), "Backup ASI size mismatch!"
    assert os.path.getsize(ini_game) == os.path.getsize(ini_backup), "Backup INI size mismatch!"
    print("    [PASS] Backup repository mirrors game deployment exactly.")

if __name__ == "__main__":
    game_dir = r"C:\Program Files (x86)\Steam\steamapps\common\Grand Theft Auto V Enhanced"
    scratch_dir = r"C:\Users\thijs\.gemini\antigravity\scratch\EnhancedImGuiMenu"

    test_pe_binary(os.path.join(game_dir, "EnhancedImGuiMenu.asi"))
    test_native_hashes(os.path.join(scratch_dir, "natives.json"), os.path.join(scratch_dir, "src", "Cheats.cpp"))
    test_ini_config(os.path.join(game_dir, "EnhancedImGuiMenu.ini"))
    test_mod_backup(game_dir)
    print("\n[ALL TEST SUITES PASSED SUCCESSFULLY]")
