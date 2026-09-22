using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace GTAVCli
{
    public class SaveSnapshot
    {
        public string Id        { get; set; } = Guid.NewGuid().ToString("N")[..8];
        public string Name      { get; set; } = "";
        public DateTime Created { get; set; } = DateTime.Now;
        public string Path      { get; set; } = "";
        public int FileCount    { get; set; }
        public long TotalBytes  { get; set; }

        public string SizeDisplay => TotalBytes < 1024 * 1024
            ? $"{TotalBytes / 1024.0:F1} KB"
            : $"{TotalBytes / 1024.0 / 1024.0:F2} MB";

        public string DateDisplay => Created.ToString("yyyy-MM-dd HH:mm");
    }

    public static class SavegameManager
    {
        private static readonly string[] SaveSubDirs = { "", "stats" };

        public static string GetProfilesDirectory()
            => Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments),
                "Rockstar Games", "GTAV Enhanced", "Profiles");

        public static string GetVaultDirectory()
            => Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments),
                "Rockstar Games", "GTAV Enhanced", "SavegameVault");

        public static List<string> GetProfiles()
        {
            string dir = GetProfilesDirectory();
            if (!Directory.Exists(dir)) return new();
            return Directory.GetDirectories(dir)
                .Select(Path.GetFileName)
                .Where(n => !string.IsNullOrEmpty(n))
                .Select(n => n!)
                .ToList();
        }

        public static SaveSnapshot CreateSnapshot(string profileId, string name)
        {
            string srcDir  = Path.Combine(GetProfilesDirectory(), profileId);
            string vault   = Path.Combine(GetVaultDirectory(), profileId);
            string snapId  = DateTime.Now.ToString("yyyyMMdd_HHmmss") + "_" + name.Replace(" ", "_");
            string destDir = Path.Combine(vault, snapId);

            Directory.CreateDirectory(destDir);

            var files = Directory.GetFiles(srcDir, "*", SearchOption.AllDirectories);
            long totalBytes = 0;
            foreach (var f in files)
            {
                string rel  = Path.GetRelativePath(srcDir, f);
                string dest = Path.Combine(destDir, rel);
                Directory.CreateDirectory(Path.GetDirectoryName(dest)!);
                File.Copy(f, dest, overwrite: true);
                totalBytes += new FileInfo(f).Length;
            }

            return new SaveSnapshot
            {
                Name       = name,
                Created    = DateTime.Now,
                Path       = destDir,
                FileCount  = files.Length,
                TotalBytes = totalBytes
            };
        }

        public static List<SaveSnapshot> GetSnapshots(string profileId)
        {
            string vault = Path.Combine(GetVaultDirectory(), profileId);
            if (!Directory.Exists(vault)) return new();

            var snaps = new List<SaveSnapshot>();
            foreach (var dir in Directory.GetDirectories(vault).OrderByDescending(d => d))
            {
                var files = Directory.GetFiles(dir, "*", SearchOption.AllDirectories);
                snaps.Add(new SaveSnapshot
                {
                    Name       = Path.GetFileName(dir),
                    Created    = Directory.GetCreationTime(dir),
                    Path       = dir,
                    FileCount  = files.Length,
                    TotalBytes = files.Sum(f => new FileInfo(f).Length)
                });
            }
            return snaps;
        }

        public static void RestoreSnapshot(string profileId, SaveSnapshot snap)
        {
            string destDir = Path.Combine(GetProfilesDirectory(), profileId);
            Directory.CreateDirectory(destDir);

            foreach (var f in Directory.GetFiles(snap.Path, "*", SearchOption.AllDirectories))
            {
                string rel  = Path.GetRelativePath(snap.Path, f);
                string dest = Path.Combine(destDir, rel);
                Directory.CreateDirectory(Path.GetDirectoryName(dest)!);
                File.Copy(f, dest, overwrite: true);
            }
        }
    }
}
