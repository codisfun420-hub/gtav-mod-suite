using System;
using System.Diagnostics;
using System.IO;

namespace GTAVCli
{
    public static class GameLauncher
    {
        public static bool Launch(GameProfile profile)
        {
            if (!profile.IsDetected || string.IsNullOrEmpty(profile.Path)) return false;

            string steamExe = @"C:\Program Files (x86)\Steam\steam.exe";

            // Try Steam -applaunch with BattlEye bypass
            if (File.Exists(steamExe))
            {
                try
                {
                    Process.Start(new ProcessStartInfo
                    {
                        FileName = steamExe,
                        Arguments = $"-applaunch {profile.SteamAppId} -nobattleye -noBE",
                        UseShellExecute = true
                    });
                    return true;
                }
                catch { }
            }

            // Try steam:// URI
            try
            {
                Process.Start(new ProcessStartInfo
                {
                    FileName = $"steam://run/{profile.SteamAppId}//-nobattleye%20-noBE/",
                    UseShellExecute = true
                });
                return true;
            }
            catch { }

            // Fallback: launch executable directly
            string playExe  = Path.Combine(profile.Path, "PlayGTAV.exe");
            string gameExe  = Path.Combine(profile.Path, profile.ExecutableName);
            string target   = File.Exists(playExe) ? playExe : gameExe;

            if (File.Exists(target))
            {
                Process.Start(new ProcessStartInfo
                {
                    FileName = target,
                    Arguments = "-nobattleye -noBE",
                    WorkingDirectory = profile.Path,
                    UseShellExecute = true
                });
                return true;
            }

            return false;
        }
    }
}
