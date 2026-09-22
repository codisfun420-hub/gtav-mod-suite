using System;
using System.IO;
using System.Text.RegularExpressions;

namespace GTAVCli
{
    public class CrashReport
    {
        public bool HasDump          { get; set; }
        public string? DumpFile      { get; set; }
        public string? ExceptionCode { get; set; }
        public string? FaultAddress  { get; set; }
        public string? Diagnosis     { get; set; }
        public string? Recommendation{ get; set; }
        public bool    IsAutoHealed  { get; set; }
        public int     HealCount     { get; set; }
    }

    public static class CrashAnalyzer
    {
        private static readonly string[] KnownAutoHealOffsets =
        {
            "1E963D2", "1E9645C", "1E7A330", "1E7F978", "1E97340"
        };

        public static CrashReport Analyze(GameProfile profile)
        {
            string dir     = profile.Path;
            string logFile = Path.Combine(dir, "EnhancedImGuiMenu.log");
            string dmpFile = Path.Combine(dir, "EnhancedImGuiMenu_crash.dmp");

            var report = new CrashReport
            {
                HasDump = File.Exists(dmpFile),
                DumpFile = File.Exists(dmpFile) ? dmpFile : null
            };

            if (!File.Exists(logFile))
            {
                report.Diagnosis      = "Geen EnhancedImGuiMenu.log gevonden.";
                report.Recommendation = "Start het spel minstens één keer via de CLI launcher.";
                return report;
            }

            string log = SafeReadLog(logFile);

            // Count auto-heals
            foreach (var offset in KnownAutoHealOffsets)
                if (log.Contains(offset, StringComparison.OrdinalIgnoreCase))
                    report.HealCount++;

            report.IsAutoHealed = report.HealCount > 0;

            // Check for crash
            var crashMatch = Regex.Match(log,
                @"\[CRASH DETECTED\] Exception Code: (0x[0-9A-Fa-f]+) in GTA5_Enhanced\.exe \(\+0x([0-9A-Fa-f]+)\)",
                RegexOptions.RightToLeft);

            if (crashMatch.Success)
            {
                report.ExceptionCode = crashMatch.Groups[1].Value;
                report.FaultAddress  = "+0x" + crashMatch.Groups[2].Value;
                string offset        = crashMatch.Groups[2].Value.ToUpperInvariant();

                bool knownOffset = Array.Exists(KnownAutoHealOffsets,
                    o => o.Equals(offset, StringComparison.OrdinalIgnoreCase));

                if (knownOffset)
                {
                    report.Diagnosis = $"Crash op bekend Steam-interface offset (+0x{offset}). " +
                                       "Auto-Heal handler aanwezig maar nog niet actief in deze run.";
                    report.Recommendation = "Herstart het spel — de fix is gedeployed en wordt bij de volgende launch actief.";
                }
                else
                {
                    report.Diagnosis = $"Nieuwe crashlocatie: +0x{offset} (0xC0000005 = null pointer dereference). " +
                                       "Waarschijnlijk een 5e Steam-interface callback.";
                    report.Recommendation = "Stuur het logbestand naar de ontwikkelaar voor analyse.";
                }
            }
            else if (report.IsAutoHealed)
            {
                report.Diagnosis = $"Alle {report.HealCount} bekende Steam-crashes zijn automatisch hersteld. Geen nieuwe crash gedetecteerd.";
                report.Recommendation = "Het spel draait stabiel.";
            }
            else
            {
                report.Diagnosis      = "Geen crashinformatie gevonden in het logbestand.";
                report.Recommendation = "Start het spel om een nieuwe sessie te loggen.";
            }

            return report;
        }

        private static string SafeReadLog(string path)
        {
            try
            {
                using var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
                using var sr = new StreamReader(fs);
                return sr.ReadToEnd();
            }
            catch { return ""; }
        }
    }
}
