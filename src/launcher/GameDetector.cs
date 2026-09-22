using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text.RegularExpressions;
using Microsoft.Win32;

namespace GTAVCli
{
    public enum GameEdition { Enhanced, Legacy }
    public enum ModMode { ModdedStory, VanillaSafe }

    public class GameProfile
    {
        public GameEdition Edition { get; set; }
        public string Title { get; set; } = "";
        public string ExecutableName { get; set; } = "";
        public string SteamAppId { get; set; } = "";
        public string GraphicsApi { get; set; } = "";
        public string Path { get; set; } = "";
        public bool IsDetected { get; set; }
        public bool IsRunning { get; set; }
        public string Version { get; set; } = "";
        public ModMode CurrentMode { get; set; }
    }

    public class ModEntry
    {
        public string Name { get; set; } = "";
        public string FileName { get; set; } = "";   // e.g. "EnhancedImGuiMenu.asi"
        public string OpenKey { get; set; } = "";
        public bool AlwaysOn { get; set; }            // e.g. DirectStorageFix
        public bool EnhancedOnly { get; set; }
        public bool LegacyOnly { get; set; }
    }

    public static class GameDetector
    {
        public static (GameProfile enhanced, GameProfile legacy) DetectAll()
        {
            var enhanced = new GameProfile
            {
                Edition = GameEdition.Enhanced,
                Title = "Grand Theft Auto V Enhanced Edition",
                ExecutableName = "GTA5_Enhanced.exe",
                SteamAppId = "3240220",
                GraphicsApi = "DirectX 12"
            };
            var legacy = new GameProfile
            {
                Edition = GameEdition.Legacy,
                Title = "Grand Theft Auto V Legacy Edition",
                ExecutableName = "GTA5.exe",
                SteamAppId = "271590",
                GraphicsApi = "DirectX 11"
            };

            foreach (var path in GetCandidatePaths())
            {
                if (string.IsNullOrWhiteSpace(path) || !Directory.Exists(path)) continue;

                if (string.IsNullOrEmpty(enhanced.Path))
                {
                    string exe = System.IO.Path.Combine(path, enhanced.ExecutableName);
                    if (File.Exists(exe))
                    {
                        enhanced.Path = path;
                        enhanced.IsDetected = true;
                        enhanced.Version = GetFileVersion(exe);
                    }
                }
                if (string.IsNullOrEmpty(legacy.Path))
                {
                    string exe = System.IO.Path.Combine(path, legacy.ExecutableName);
                    if (File.Exists(exe) && !File.Exists(System.IO.Path.Combine(path, "GTA5_Enhanced.exe")))
                    {
                        legacy.Path = path;
                        legacy.IsDetected = true;
                        legacy.Version = GetFileVersion(exe);
                    }
                }
            }

            UpdateRunning(enhanced);
            UpdateRunning(legacy);
            return (enhanced, legacy);
        }

        public static void UpdateRunning(GameProfile p)
        {
            if (!p.IsDetected) { p.IsRunning = false; return; }
            string proc = System.IO.Path.GetFileNameWithoutExtension(p.ExecutableName);
            p.IsRunning = Process.GetProcessesByName(proc).Length > 0;
        }

        private static List<string> GetCandidatePaths()
        {
            var paths = new List<string>
            {
                @"C:\Program Files (x86)\Steam\steamapps\common\Grand Theft Auto V Enhanced",
                @"C:\Program Files (x86)\Steam\steamapps\common\Grand Theft Auto V",
                @"C:\Program Files\Rockstar Games\Grand Theft Auto V",
                @"D:\SteamLibrary\steamapps\common\Grand Theft Auto V Enhanced",
                @"D:\SteamLibrary\steamapps\common\Grand Theft Auto V",
                @"E:\SteamLibrary\steamapps\common\Grand Theft Auto V Enhanced",
                @"E:\SteamLibrary\steamapps\common\Grand Theft Auto V"
            };
            try
            {
                string? steamPath = Registry.GetValue(@"HKEY_CURRENT_USER\Software\Valve\Steam", "SteamPath", null) as string;
                if (!string.IsNullOrEmpty(steamPath))
                {
                    string vdf = System.IO.Path.Combine(steamPath, "steamapps", "libraryfolders.vdf");
                    if (File.Exists(vdf))
                    {
                        var ms = Regex.Matches(File.ReadAllText(vdf), @"""path""\s*""([^""]+)""");
                        foreach (Match m in ms)
                        {
                            string lib = m.Groups[1].Value.Replace(@"\\", @"\");
                            string common = System.IO.Path.Combine(lib, "steamapps", "common");
                            if (Directory.Exists(common))
                            {
                                paths.Add(System.IO.Path.Combine(common, "Grand Theft Auto V Enhanced"));
                                paths.Add(System.IO.Path.Combine(common, "Grand Theft Auto V"));
                            }
                        }
                    }
                }
            }
            catch { }
            return paths;
        }

        private static string GetFileVersion(string path)
        {
            try { var v = FileVersionInfo.GetVersionInfo(path); return v.FileVersion ?? v.ProductVersion ?? "?"; }
            catch { return "?"; }
        }
    }
}
