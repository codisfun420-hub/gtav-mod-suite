using System;
using System.Diagnostics;
using System.IO;
using System.Threading;

namespace GTAVCli
{
    public enum StoryModeState
    {
        NotRunning,
        Launching,
        Loading,
        StoryMode,
        Exited
    }

    public static class StoryModeDetector
    {
        /// <summary>
        /// Wacht tot GTA V Story Mode bereikt is.
        /// Rapporteert voortgang via de callback.
        /// Returns true als story mode gedetecteerd is, false als het spel crashed/afgesloten.
        /// </summary>
        public static bool WaitForStoryMode(
            GameProfile profile,
            Action<StoryModeState, string> onProgress,
            CancellationToken ct,
            int timeoutSeconds = 300)
        {
            string procName = Path.GetFileNameWithoutExtension(profile.ExecutableName);
            string logFile  = Path.Combine(profile.Path, "EnhancedImGuiMenu.log");
            string shkFile  = Path.Combine(profile.Path, "ScriptHookV.log");

            var sw = Stopwatch.StartNew();
            var sessionStart = DateTime.Now;
            bool processFound = false;

            // Record log file timestamps at session start so we only read NEW content
            DateTime logBaseline = File.Exists(logFile) ? File.GetLastWriteTime(logFile) : DateTime.MinValue;
            DateTime shkBaseline = File.Exists(shkFile) ? File.GetLastWriteTime(shkFile) : DateTime.MinValue;

            onProgress(StoryModeState.Launching, "Wachten op game process...");

            while (!ct.IsCancellationRequested && sw.Elapsed.TotalSeconds < timeoutSeconds)
            {
                var procs = Process.GetProcessesByName(procName);
                if (procs.Length > 0)
                {
                    if (!processFound)
                    {
                        processFound = true;
                        onProgress(StoryModeState.Loading, $"Game gevonden (PID {procs[0].Id}). Wachten op Story Mode...");
                    }

                    // Check ScriptHookV for "Creating threads" = story mode reached
                    if (File.Exists(shkFile) && File.GetLastWriteTime(shkFile) > shkBaseline &&
                        CheckFileForPattern(shkFile, "CORE: Creating threads"))
                    {
                        onProgress(StoryModeState.StoryMode, $"Story Mode gedetecteerd via ScriptHookV ({(int)sw.Elapsed.TotalSeconds}s)");
                        return true;
                    }

                    // Check EnhancedImGuiMenu log for DX12 init = script running in story mode
                    if (File.Exists(logFile) && File.GetLastWriteTime(logFile) > logBaseline &&
                        (CheckFileForPattern(logFile, "initialization complete") ||
                         CheckFileForPattern(logFile, "ImGui DX12") ||
                         CheckFileForPattern(logFile, "Hooked game Window")))
                    {
                        onProgress(StoryModeState.StoryMode, $"Story Mode gedetecteerd via ImGui log ({(int)sw.Elapsed.TotalSeconds}s)");
                        return true;
                    }

                    // Fallback: game is running for 90+ seconds = likely in story mode
                    if (processFound && sw.Elapsed.TotalSeconds >= 90)
                    {
                        onProgress(StoryModeState.StoryMode, $"Story Mode aangenomen na {(int)sw.Elapsed.TotalSeconds}s (game actief)");
                        return true;
                    }

                    // Check if process exited (crashed)
                    if (procs[0].HasExited)
                    {
                        onProgress(StoryModeState.Exited, "Game is afgesloten of gecrasht.");
                        return false;
                    }
                }
                else if (processFound)
                {
                    // Was running, now gone
                    onProgress(StoryModeState.Exited, "Game is afgesloten.");
                    return false;
                }

                Thread.Sleep(1000);
            }

            if (!processFound)
            {
                onProgress(StoryModeState.NotRunning, "Game process niet gevonden binnen timeout.");
            }
            else
            {
                onProgress(StoryModeState.Exited, "Timeout — story mode niet gedetecteerd.");
            }
            return false;
        }

        /// <summary>
        /// Wacht tot het game process afgesloten is.
        /// </summary>
        public static void WaitForExit(GameProfile profile, Action<string> onStatus, CancellationToken ct)
        {
            string procName = Path.GetFileNameWithoutExtension(profile.ExecutableName);
            while (!ct.IsCancellationRequested)
            {
                var procs = Process.GetProcessesByName(procName);
                if (procs.Length == 0)
                {
                    onStatus("Game afgesloten.");
                    return;
                }
                Thread.Sleep(2000);
            }
        }

        private static bool CheckFileForPattern(string filePath, string pattern)
        {
            if (!File.Exists(filePath)) return false;
            try
            {
                // Read only last 8KB to avoid large file reads
                using var fs = new FileStream(filePath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
                long seek = Math.Max(0, fs.Length - 8192);
                fs.Seek(seek, SeekOrigin.Begin);
                using var sr = new StreamReader(fs);
                string tail = sr.ReadToEnd();
                return tail.Contains(pattern, StringComparison.OrdinalIgnoreCase);
            }
            catch { return false; }
        }
    }
}
