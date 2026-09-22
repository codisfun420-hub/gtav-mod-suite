using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;
using GTAVCli;

Console.OutputEncoding = System.Text.Encoding.UTF8;
Console.Title = "GTA V Mod Launcher  v3.0";

try { Console.SetWindowSize(Math.Min(Console.LargestWindowWidth, 90), Math.Min(Console.LargestWindowHeight, 42)); } catch { }
try { Console.BufferWidth = 90; } catch { }

// ── Boot ──────────────────────────────────────────────────────────────────────
UI.Splash();
Console.Write("  Detecting installations");
var (enhanced, legacy) = GameDetector.DetectAll();
UI.DotTick(3);
Console.WriteLine();

var profiles = new List<GameProfile>();
if (enhanced.IsDetected) profiles.Add(enhanced);
if (legacy.IsDetected)   profiles.Add(legacy);

if (profiles.Count == 0)
{
    Console.WriteLine();
    UI.Alert("No GTA V installation found. Make sure Steam is running.", AlertKind.Err);
    UI.Pause(); return;
}

Thread.Sleep(250);

// ── Main loop ─────────────────────────────────────────────────────────────────
while (true)
{
    Console.Clear();
    UI.Splash();
    GameDetector.UpdateRunning(profiles[0]);
    if (profiles.Count > 1) GameDetector.UpdateRunning(profiles[1]);

    UI.Section("INSTALLATIONS");
    for (int i = 0; i < profiles.Count; i++)
        UI.EditionRow(i + 1, profiles[i]);

    UI.Divider();
    UI.MenuRow("S", "Savegame Vault");
    UI.MenuRow("C", "Crash Report");
    UI.MenuRow("0", "Exit");
    UI.Bottom();

    char k = UI.Prompt();
    if (k == '0') { UI.Bye(); break; }
    if (k == 'S') { SavegameMenu(profiles); continue; }
    if (k == 'C') { CrashMenu(profiles);    continue; }
    int sel = k - '1';
    if (sel >= 0 && sel < profiles.Count) RunEditionFlow(profiles[sel]);
}

// ─────────────────────────────────────────────────────────────────────────────
//  FLOW
// ─────────────────────────────────────────────────────────────────────────────

static void RunEditionFlow(GameProfile profile)
{
    Console.Clear();
    UI.Splash();

    string tag = profile.Edition == GameEdition.Enhanced ? "ENHANCED  /  DX12" : "LEGACY  /  DX11";
    UI.Section($"SELECT MOD MENU  --  {tag}");

    // Status table
    var statusList = AsiManager.GetStatus(profile);
    Console.WriteLine();
    foreach (var (mod, active, found) in statusList)
    {
        string state = !found ? "N/A" : active ? "ON " : "OFF";
        ConsoleColor c = !found ? ConsoleColor.DarkGray : active ? ConsoleColor.Cyan : ConsoleColor.DarkGray;
        Console.Write("    ");
        Console.ForegroundColor = c;
        Console.Write($"[{state}]");
        Console.ResetColor();
        Console.ForegroundColor = !found || !active ? ConsoleColor.DarkGray : ConsoleColor.White;
        Console.WriteLine($"  {mod.FileName}");
        Console.ResetColor();
    }

    UI.Divider();
    Console.WriteLine();

    var mods = AsiManager.GetSelectableMods(profile);
    for (int i = 0; i < mods.Count; i++)
    {
        bool found = statusList.Any(s => s.mod.FileName == mods[i].FileName && s.found);
        UI.ModRow(i + 1, mods[i], found);
    }
    Console.WriteLine();
    UI.MenuRow("N", "No mod menu  (crash fixes only)");
    UI.MenuRow("0", "Back");
    UI.Bottom();

    char k = UI.Prompt();
    if (k == '0') return;

    ModEntry? selected = null;
    if (k != 'N')
    {
        int idx = k - '1';
        if (idx < 0 || idx >= mods.Count) return;
        selected = mods[idx];
        if (!statusList.Any(s => s.mod.FileName == selected.FileName && s.found))
        {
            Console.WriteLine();
            UI.Alert($"{selected.FileName} is not installed in {profile.Path}", AlertKind.Err);
            UI.Pause(); return;
        }
    }

    // Confirm
    Console.Clear();
    UI.Splash();
    UI.Section("CONFIRM");
    Console.WriteLine();
    UI.KV("Edition",  profile.Title);
    UI.KV("Mod",      selected?.Name ?? "None  (crash fixes only)");
    UI.KV("Path",     profile.Path);
    Console.WriteLine();

    // Apply
    UI.Section("APPLYING CHANGES");
    Console.WriteLine();
    AsiManager.ApplySelection(profile, selected);
    Console.WriteLine();

    // Launch
    UI.Section("LAUNCHING");
    Console.WriteLine();
    bool launched = GameLauncher.Launch(profile);
    if (!launched)
    {
        UI.Alert("Could not start game via Steam.", AlertKind.Err);
        AsiManager.RestoreAll(profile); UI.Pause(); return;
    }
    UI.Alert("Game launched via Steam.", AlertKind.Ok);
    Console.WriteLine();

    // Wait
    UI.Section("WAITING FOR STORY MODE");
    Console.WriteLine("    Press Q to cancel.\n");

    using var cts = new CancellationTokenSource();
    _ = System.Threading.Tasks.Task.Run(() =>
    {
        while (!cts.Token.IsCancellationRequested)
        {
            if (Console.KeyAvailable && Console.ReadKey(true).Key == ConsoleKey.Q)
            { cts.Cancel(); break; }
            Thread.Sleep(80);
        }
    });

    int spinRow = Console.CursorTop;
    _ = System.Threading.Tasks.Task.Run(() =>
    {
        char[] f = { '|', '/', '-', '\\' };
        int i = 0;
        while (!cts.Token.IsCancellationRequested)
        {
            try
            {
                Console.SetCursorPosition(4, spinRow);
                Console.ForegroundColor = ConsoleColor.Cyan;
                Console.Write(f[i++ % 4]);
                Console.ResetColor();
            }
            catch { }
            Thread.Sleep(120);
        }
    });

    bool ok = StoryModeDetector.WaitForStoryMode(
        profile,
        onProgress: (state, msg) =>
        {
            try
            {
                Console.SetCursorPosition(6, spinRow);
                Console.ForegroundColor = state == StoryModeState.StoryMode ? ConsoleColor.Green
                                        : state == StoryModeState.Exited     ? ConsoleColor.Red
                                        : ConsoleColor.DarkGray;
                Console.Write($"{msg,-72}");
                Console.ResetColor();
            }
            catch { }
        },
        ct: cts.Token);

    cts.Cancel();
    Thread.Sleep(130);
    Console.WriteLine("\n");

    if (ok)
    {
        UI.BigSuccess(selected);
        Console.WriteLine("\n    Press Q to stop monitoring and restore files.\n");

        using var cts2 = new CancellationTokenSource();
        bool quit = false;
        var t = System.Threading.Tasks.Task.Run(() =>
            StoryModeDetector.WaitForExit(profile, m => Console.WriteLine($"\n    {m}"), cts2.Token));
        while (!t.IsCompleted)
        {
            if (Console.KeyAvailable && Console.ReadKey(true).Key == ConsoleKey.Q)
            { quit = true; cts2.Cancel(); break; }
            Thread.Sleep(150);
        }
        if (!quit) Console.WriteLine("\n    Game closed.");
    }
    else
    {
        Console.WriteLine();
        UI.Alert("Story Mode not reached -- game crashed or exited.", AlertKind.Err);
        Console.WriteLine();
        var r = CrashAnalyzer.Analyze(profile);
        if (r.HealCount > 0 || r.ExceptionCode != null)
        {
            UI.Section("CRASH REPORT");
            Console.WriteLine();
            string healBar = new string('#', r.HealCount * 5) + new string('.', (4 - r.HealCount) * 5);
            UI.KV("Auto-heal", $"[{healBar}]  {r.HealCount}/4");
            UI.KV("Address",   r.FaultAddress  ?? "none");
            UI.KV("Diagnosis", r.Diagnosis     ?? "--");
            UI.KV("Advice",    r.Recommendation ?? "--");
            if (r.HasDump) UI.KV("Dump", r.DumpFile ?? "");
            Console.WriteLine();
        }
    }

    UI.Section("RESTORING FILES");
    Console.WriteLine();
    AsiManager.RestoreAll(profile);
    Console.WriteLine();
    UI.Pause();
}

// ─────────────────────────────────────────────────────────────────────────────
//  SAVEGAME
// ─────────────────────────────────────────────────────────────────────────────

static void SavegameMenu(List<GameProfile> profiles)
{
    var ids = SavegameManager.GetProfiles();
    if (ids.Count == 0)
    {
        Console.Clear(); UI.Splash();
        UI.Alert("No save profiles found.", AlertKind.Warn);
        UI.Pause(); return;
    }
    string pid = ids[0];

    while (true)
    {
        Console.Clear(); UI.Splash();
        UI.Section($"SAVEGAME VAULT  --  {pid}");
        Console.WriteLine();

        var snaps = SavegameManager.GetSnapshots(pid);
        if (snaps.Count == 0)
        {
            Console.ForegroundColor = ConsoleColor.DarkGray;
            Console.WriteLine("    No snapshots found.\n");
            Console.ResetColor();
        }
        else
        {
            Console.ForegroundColor = ConsoleColor.DarkGray;
            Console.WriteLine($"    {"#",-4}  {"Date",-18}  {"Name",-28}  {"Size",-10}  Files");
            Console.WriteLine($"    {"----",-4}  {"------------------",-18}  {"----------------------------",-28}  {"----------",-10}  -----");
            Console.ResetColor();
            for (int i = 0; i < Math.Min(snaps.Count, 10); i++)
            {
                var s = snaps[i];
                Console.ForegroundColor = ConsoleColor.DarkGray;
                Console.Write($"    [{i + 1}]  ");
                Console.ResetColor();
                Console.Write($"{s.DateDisplay,-18}  ");
                Console.ForegroundColor = ConsoleColor.White;
                Console.Write($"{s.Name,-28}");
                Console.ResetColor();
                Console.ForegroundColor = ConsoleColor.DarkGray;
                Console.WriteLine($"  {s.SizeDisplay,-10}  {s.FileCount}");
                Console.ResetColor();
            }
            Console.WriteLine();
        }

        UI.Divider();
        UI.MenuRow("C", "Create snapshot");
        if (snaps.Count > 0) UI.MenuRow("R", "Restore snapshot");
        UI.MenuRow("0", "Back");
        UI.Bottom();

        char k = UI.Prompt();
        if (k == '0') break;

        if (k == 'C')
        {
            Console.Write("\n    Snapshot name: ");
            Console.ForegroundColor = ConsoleColor.White;
            string name = Console.ReadLine()?.Trim() ?? "backup";
            Console.ResetColor();
            if (string.IsNullOrEmpty(name)) name = "backup";
            Console.Write("    Creating");
            var snap = SavegameManager.CreateSnapshot(pid, name);
            UI.DotTick(3);
            Console.WriteLine();
            UI.Alert($"Snapshot '{snap.Name}'  ({snap.SizeDisplay}, {snap.FileCount} files)", AlertKind.Ok);
            UI.Pause();
        }
        else if (k == 'R' && snaps.Count > 0)
        {
            Console.Write($"\n    Restore # [1-{snaps.Count}]: ");
            Console.ForegroundColor = ConsoleColor.White;
            string? inp = Console.ReadLine();
            Console.ResetColor();
            if (int.TryParse(inp, out int n) && n >= 1 && n <= snaps.Count)
            {
                Console.Write($"    Restore '{snaps[n-1].Name}'? [y/N]: ");
                Console.ForegroundColor = ConsoleColor.Yellow;
                string? c = Console.ReadLine();
                Console.ResetColor();
                if (c?.Trim().Equals("y", StringComparison.OrdinalIgnoreCase) == true ||
                    c?.Trim().Equals("j", StringComparison.OrdinalIgnoreCase) == true)
                {
                    SavegameManager.RestoreSnapshot(pid, snaps[n-1]);
                    UI.Alert("Snapshot restored.", AlertKind.Ok);
                }
            }
            UI.Pause();
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  CRASH
// ─────────────────────────────────────────────────────────────────────────────

static void CrashMenu(List<GameProfile> profiles)
{
    Console.Clear(); UI.Splash();
    UI.Section("CRASH REPORT");
    Console.WriteLine();

    foreach (var p in profiles.Where(x => x.IsDetected))
    {
        var r = CrashAnalyzer.Analyze(p);
        string ed = p.Edition == GameEdition.Enhanced ? "Enhanced" : "Legacy";

        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.WriteLine($"  +-- {ed} {new string('-', 60 - ed.Length)}+");
        Console.ResetColor();

        string healBar = new string('#', r.HealCount * 5) + new string('.', (4 - r.HealCount) * 5);
        Console.ForegroundColor = r.HealCount == 4 ? ConsoleColor.Green : ConsoleColor.Yellow;
        Console.WriteLine($"  |  Auto-Heal   [{healBar}]  {r.HealCount}/4 crashes intercepted");
        Console.ResetColor();
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.WriteLine($"  |  Address     {r.FaultAddress ?? "none"}");
        Console.WriteLine($"  |  Diagnosis   {r.Diagnosis ?? "--"}");
        Console.WriteLine($"  |  Advice      {r.Recommendation ?? "--"}");
        Console.WriteLine($"  |  Dump        {(r.HasDump ? "[present]  " + r.DumpFile : "not found")}");
        Console.WriteLine($"  +{new string('-', 64)}+");
        Console.ResetColor();
        Console.WriteLine();
    }

    UI.Pause();
}

// ─────────────────────────────────────────────────────────────────────────────
//  UI
// ─────────────────────────────────────────────────────────────────────────────

public enum AlertKind { Ok, Warn, Err, Info }

public static class UI
{
    private const int W = 86;

    // ── Logo ──────────────────────────────────────────────────────────────────

    public static void Splash()
    {
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.WriteLine();
        Console.ForegroundColor = ConsoleColor.Cyan;
        Console.WriteLine(@"  _________________________________________ ___    __");
        Console.WriteLine(@"  __  ____/__  __/__    |__  ____/__  __ )/   |  / /");
        Console.ForegroundColor = ConsoleColor.White;
        Console.WriteLine(@"  _  / __ __  /  __  /| |_  / __ __  /_/ / /| | /_/ ");
        Console.ForegroundColor = ConsoleColor.Cyan;
        Console.WriteLine(@"  / /_/ / _  /   _  ___ |/ /_/ / _  _, _/ ___ |_   ");
        Console.WriteLine(@"  \____/  /_/    /_/  |_|\____/  /_/ |_/_/  |_|/_/ ");
        Console.ResetColor();
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.WriteLine($"  {new string('-', W)}");
        Console.Write("  MOD LAUNCHER");
        Console.ForegroundColor = ConsoleColor.Cyan;
        Console.Write("  v3.0");
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.WriteLine("    Enhanced Edition  /  Legacy Edition    Story Mode Selector");
        Console.WriteLine($"  {new string('-', W)}");
        Console.ResetColor();
        Console.WriteLine();
    }

    // ── Structure ─────────────────────────────────────────────────────────────

    public static void Section(string title)
    {
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.Write($"  +-- ");
        Console.ForegroundColor = ConsoleColor.Yellow;
        Console.Write(title);
        Console.ForegroundColor = ConsoleColor.DarkGray;
        int rem = W - 5 - title.Length;
        Console.WriteLine(rem > 0 ? new string('-', rem) : "");
        Console.ResetColor();
        Console.WriteLine();
    }

    public static void Divider()
    {
        Console.WriteLine();
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.WriteLine($"  +{new string('-', W - 2)}+");
        Console.ResetColor();
        Console.WriteLine();
    }

    public static void Bottom()
    {
        Console.WriteLine();
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.WriteLine($"  {new string('-', W)}");
        Console.ResetColor();
    }

    // ── Rows ──────────────────────────────────────────────────────────────────

    public static void EditionRow(int n, GameProfile p)
    {
        string api = p.Edition == GameEdition.Enhanced ? "DX12" : "DX11";

        Console.Write("  ");
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.Write("  [");
        Console.ForegroundColor = ConsoleColor.White;
        Console.Write(n);
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.Write("]  ");
        Console.ForegroundColor = p.Edition == GameEdition.Enhanced ? ConsoleColor.Cyan : ConsoleColor.White;
        Console.Write($"{p.Title,-43}");
        Console.ResetColor();
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.Write($"  {api}   v{p.Version}");
        if (p.IsRunning)
        {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.Write("   [ACTIVE]");
        }
        Console.ResetColor();
        Console.WriteLine();
    }

    public static void MenuRow(string key, string label)
    {
        Console.Write("  ");
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.Write("  [");
        Console.ForegroundColor = ConsoleColor.White;
        Console.Write(key);
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.Write("]  ");
        Console.ForegroundColor = ConsoleColor.White;
        Console.WriteLine(label);
        Console.ResetColor();
    }

    public static void ModRow(int n, ModEntry mod, bool installed)
    {
        Console.Write("  ");
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.Write("  [");
        Console.ForegroundColor = installed ? ConsoleColor.White : ConsoleColor.DarkGray;
        Console.Write(n);
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.Write("]  ");
        Console.ForegroundColor = installed ? ConsoleColor.Cyan : ConsoleColor.DarkGray;
        Console.Write($"{mod.Name,-46}");
        Console.ResetColor();
        if (installed)
        {
            Console.ForegroundColor = ConsoleColor.DarkGray;
            Console.Write("  key: ");
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.Write(mod.OpenKey);
        }
        else
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.Write("  [NOT INSTALLED]");
        }
        Console.ResetColor();
        Console.WriteLine();
    }

    // ── Alerts ────────────────────────────────────────────────────────────────

    public static void Alert(string msg, AlertKind kind)
    {
        string tag = kind switch
        {
            AlertKind.Ok   => " OK  ",
            AlertKind.Warn => "WARN ",
            AlertKind.Err  => " ERR ",
            _              => "INFO ",
        };
        ConsoleColor bg = kind switch
        {
            AlertKind.Ok   => ConsoleColor.DarkGreen,
            AlertKind.Warn => ConsoleColor.DarkYellow,
            AlertKind.Err  => ConsoleColor.DarkRed,
            _              => ConsoleColor.DarkBlue,
        };
        ConsoleColor fg = kind == AlertKind.Warn ? ConsoleColor.Black : ConsoleColor.White;

        Console.Write("  ");
        Console.BackgroundColor = bg;
        Console.ForegroundColor = fg;
        Console.Write($" {tag}");
        Console.ResetColor();
        Console.ForegroundColor = kind switch
        {
            AlertKind.Ok   => ConsoleColor.Green,
            AlertKind.Warn => ConsoleColor.Yellow,
            AlertKind.Err  => ConsoleColor.Red,
            _              => ConsoleColor.Cyan,
        };
        Console.WriteLine($"  {msg}");
        Console.ResetColor();
    }

    public static void BigSuccess(ModEntry? mod)
    {
        string modName = mod?.Name ?? "No mod menu";
        string keyLine = mod != null ? $"  Open with: [{mod.OpenKey}]" : "  Running vanilla + crash fixes.";
        int iw = 66;
        Console.WriteLine();
        Console.ForegroundColor = ConsoleColor.Cyan;
        Console.WriteLine($"  +{new string('=', iw)}+");
        Console.WriteLine($"  |{new string(' ', iw)}|");
        Console.Write    ($"  |   STORY MODE ACTIVE");
        Console.ForegroundColor = ConsoleColor.White;
        Console.Write    ($"  --  {modName}");
        int pad1 = iw - 20 - modName.Length - 6;
        Console.ForegroundColor = ConsoleColor.Cyan;
        Console.WriteLine($"{new string(' ', Math.Max(0, pad1))}|");
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.Write    ($"  |{keyLine}");
        int pad2 = iw - keyLine.Length;
        Console.WriteLine($"{new string(' ', Math.Max(0, pad2))}|");
        Console.ForegroundColor = ConsoleColor.Cyan;
        Console.WriteLine($"  |{new string(' ', iw)}|");
        Console.WriteLine($"  +{new string('=', iw)}+");
        Console.ResetColor();
    }

    // ── KV ────────────────────────────────────────────────────────────────────

    public static void KV(string key, string value)
    {
        Console.Write("  ");
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.Write($"  {key,-12}  ");
        Console.ForegroundColor = ConsoleColor.White;
        Console.WriteLine(value);
        Console.ResetColor();
    }

    // ── Input ─────────────────────────────────────────────────────────────────

    public static char Prompt()
    {
        Console.WriteLine();
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.Write("  >> ");
        Console.ForegroundColor = ConsoleColor.White;
        string? line = Console.ReadLine()?.Trim().ToUpperInvariant();
        Console.ResetColor();
        return string.IsNullOrEmpty(line) ? '\0' : line[0];
    }

    public static void Pause()
    {
        Console.WriteLine();
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.Write("  Press Enter to continue...");
        Console.ResetColor();
        Console.ReadLine();
    }

    public static void Bye()
    {
        Console.Clear();
        Splash();
        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.WriteLine("  Shutting down.\n");
        Console.ResetColor();
        Thread.Sleep(400);
    }

    // ── Util ──────────────────────────────────────────────────────────────────

    public static void DotTick(int n)
    {
        for (int i = 0; i < n; i++)
        {
            Thread.Sleep(160);
            Console.ForegroundColor = ConsoleColor.DarkGray;
            Console.Write(".");
            Console.ResetColor();
        }
    }
}
