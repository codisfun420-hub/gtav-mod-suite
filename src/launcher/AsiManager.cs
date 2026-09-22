using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace GTAVCli
{
    /// <summary>
    /// Beheert het in/uitschakelen van ASI mod bestanden per editie.
    /// Niet-geselecteerde mods worden hernoemd van .asi naar .asi.off (en terug).
    /// </summary>
    public static class AsiManager
    {
        // Alle bekende mod menu ASI bestanden (niet DirectStorageFix — die is altijd aan)
        public static readonly List<ModEntry> KnownMods = new()
        {
            new ModEntry
            {
                Name     = "Enhanced ImGui Menu (custom, DX12)",
                FileName = "EnhancedImGuiMenu.asi",
                OpenKey  = "INSERT",
                EnhancedOnly = true
            },
            new ModEntry
            {
                Name     = "Native Trainer",
                FileName = "NativeTrainer.asi",
                OpenKey  = "F4"
            },
            new ModEntry
            {
                Name     = "Rampage Trainer",
                FileName = "Rampage.asi",
                OpenKey  = "F4"
            },
            new ModEntry
            {
                Name     = "DirectStorageFix (crash-fix, always on)",
                FileName = "DirectStorageFix.asi",
                AlwaysOn = true,
                EnhancedOnly = true
            }
        };

        public static List<ModEntry> GetSelectableMods(GameProfile profile)
        {
            return KnownMods
                .Where(m => !m.AlwaysOn)
                .Where(m => !m.EnhancedOnly || profile.Edition == GameEdition.Enhanced)
                .Where(m => !m.LegacyOnly   || profile.Edition == GameEdition.Legacy)
                .ToList();
        }

        /// <summary>
        /// Zet de geselecteerde mod AAN, alle anderen UIT.
        /// AlwaysOn mods worden nooit uitgeschakeld.
        /// </summary>
        public static void ApplySelection(GameProfile profile, ModEntry? selected)
        {
            string dir = profile.Path;
            var selectable = GetSelectableMods(profile);

            foreach (var mod in selectable)
            {
                string asiPath = Path.Combine(dir, mod.FileName);
                string offPath = Path.Combine(dir, mod.FileName + ".off");

                bool shouldBeOn = selected != null && mod.FileName == selected.FileName;

                if (shouldBeOn)
                {
                    // Ensure .asi exists (rename back from .off if needed)
                    if (!File.Exists(asiPath) && File.Exists(offPath))
                    {
                        File.Move(offPath, asiPath);
                        Console.WriteLine($"  [✓] {mod.FileName} ingeschakeld");
                    }
                    else if (File.Exists(asiPath))
                    {
                        Console.WriteLine($"  [✓] {mod.FileName} al actief");
                    }
                    else
                    {
                        Console.WriteLine($"  [!] {mod.FileName} niet gevonden in {dir}");
                    }
                }
                else
                {
                    // Disable: rename .asi → .asi.off
                    if (File.Exists(asiPath))
                    {
                        File.Move(asiPath, offPath);
                        Console.WriteLine($"  [✗] {mod.FileName} uitgeschakeld (→ .off)");
                    }
                    // Already disabled
                }
            }

            // Ensure AlwaysOn mods are present
            foreach (var mod in KnownMods.Where(m => m.AlwaysOn))
            {
                bool editionMatch = (!mod.EnhancedOnly || profile.Edition == GameEdition.Enhanced) &&
                                    (!mod.LegacyOnly   || profile.Edition == GameEdition.Legacy);
                if (!editionMatch) continue;

                string asiPath = Path.Combine(dir, mod.FileName);
                string offPath = Path.Combine(dir, mod.FileName + ".off");
                if (!File.Exists(asiPath) && File.Exists(offPath))
                {
                    File.Move(offPath, asiPath);
                    Console.WriteLine($"  [✓] {mod.FileName} (always-on) hersteld");
                }
            }
        }

        /// <summary>
        /// Herstel alle .off bestanden terug naar .asi (opruimen bij afsluiten).
        /// </summary>
        public static void RestoreAll(GameProfile profile)
        {
            string dir = profile.Path;
            var offFiles = Directory.GetFiles(dir, "*.asi.off");
            foreach (var off in offFiles)
            {
                string asi = off[..^4]; // remove ".off"
                if (!File.Exists(asi))
                {
                    File.Move(off, asi);
                    Console.WriteLine($"  [↩] {Path.GetFileName(asi)} hersteld");
                }
            }
        }

        /// <summary>
        /// Geeft huidige staat van elke mod in de map terug.
        /// </summary>
        public static List<(ModEntry mod, bool active, bool found)> GetStatus(GameProfile profile)
        {
            var result = new List<(ModEntry, bool, bool)>();
            string dir = profile.Path;
            foreach (var mod in GetSelectableMods(profile))
            {
                bool active = File.Exists(Path.Combine(dir, mod.FileName));
                bool found  = active || File.Exists(Path.Combine(dir, mod.FileName + ".off"));
                result.Add((mod, active, found));
            }
            return result;
        }
    }
}
