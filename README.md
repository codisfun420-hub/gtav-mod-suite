# Grand Theft Auto V - Enhanced & Legacy Mod Suite

Dit project biedt een geïntegreerde omgeving voor het beheren en draaien van mods in Grand Theft Auto V, met ondersteuning voor zowel de Enhanced Edition (DirectX 12) als de Legacy Edition (DirectX 11).

Het systeem bestaat uit twee hoofdcomponenten:
1. Een standalone terminal-gebaseerde mod launcher en profielbeheerder (C# / .NET 8).
2. Een native DirectX 12 in-game mod menu (C++ / ImGui) voorzien van geavanceerde Vectored Exception Handling (VEH) ter bescherming tegen compatibiliteitsproblemen bij uninitialized interfaces.

---

## Architectuur en Software Structuur

Het project is opgedeeld in een modulaire indeling:

```text
gtav-mod-suite/
|-- src/
|   |-- launcher/               # C# .NET 8 Console applicatie (GTAVCli)
|   |   |-- Program.cs          # UI loop, navigatie, menusysteem en rendering
|   |   |-- AsiManager.cs       # Dynamische ASI isolatie (.asi <-> .asi.off)
|   |   |-- GameDetector.cs     # Automatische detectie van installatiepaden en edities
|   |   |-- GameLauncher.cs     # Proces initialisatie en argument injectie (-nobattleye)
|   |   |-- StoryModeDetector.cs# Real-time monitoring van laadstatus richting Story Mode
|   |   |-- CrashAnalyzer.cs    # Geautomatiseerde minidump- en log-analyse
|   |   |-- SavegameManager.cs  # Veilige snapshot- en restore functionaliteit voor saves
|   |   `-- GTAVCli.csproj      # Projectconfiguratie (.NET 8 Windows x64)
|   |
|   `-- menu-dx12/              # C++ DirectX 12 In-game Mod Menu (ASI/DLL)
|       |-- src/
|       |   |-- main.cpp        # DllMain, thread bootstrapping en shutdown lifecycle
|       |   |-- D3D12Hook.cpp   # DirectX 12 SwapChain hook & CommandQueue synchronisatie
|       |   |-- CrashHandler.cpp# VEH Auto-Heal systeem voor Steam/Arxan interface crashes
|       |   |-- Cheats.cpp      # Native hooks, speler-, voertuig- en world-logica
|       |   |-- UI.cpp          # ImGui menu structuur en render pipelines
|       |   |-- Config.cpp      # Persistentie van gebruikersinstellingen (.ini)
|       |   `-- Logger.cpp      # Thread-safe loggingsysteem
|       |-- imgui/              # Dear ImGui library inclusief DX12 & Win32 backends
|       `-- minhook/            # MinHook hooking library
|
|-- .gitignore                  # Filtert binaire bestanden, dumps en cache weg
`-- README.md                   # Technische documentatie en projectoverzicht
```

---

## Componenten in Detail

### 1. Launcher (`src/launcher`)

De launcher is ontworpen om interactieve mod-conflicten te voorkomen. In plaats van alle geïnstalleerde mods tegelijkertijd te injecteren, stelt de launcher de gebruiker in staat om per sessie één specifiek menu te selecteren.

- **AsiManager**: Hernoemt inactieve mods tijdelijk naar `.asi.off` voordat het spel start. Alleen de geselecteerde mod en essentiële runtime fixes (zoals `DirectStorageFix.asi`) blijven actief. Na afloop van de sessie worden alle bestanden automatisch in hun oorspronkelijke staat hersteld.
- **StoryModeDetector**: Monitort logfiles (`ScriptHookV.log`, `EnhancedImGuiMenu.log`) en running threads om vast te stellen wanneer het spel daadwerkelijk in Story Mode is geladen, zonder te vertrouwen op simpele tijdsvertragingen.
- **CrashAnalyzer**: Leest logboeken en minidumps uit om bekende access violation patronen (zoals `0xC0000005`) direct te identificeren en de gebruiker van duidelijke feedback te voorzien.
- **SavegameManager**: Biedt een snapshot vault waarmee game saves voor en na gemodde sessies kunnen worden veiliggesteld en hersteld.

### 2. DirectX 12 In-Game Menu (`src/menu-dx12`)

Een native 64-bit ASI-plugin specifiek gebouwd voor de Enhanced Edition van GTA V.

- **DirectX 12 Hooking**: Onderschept de SwapChain Present en CommandQueue via een dummy-device vtable inspectie om overlay-rendering synchroon te laten lopen met de engine frames.
- **VEH Auto-Heal (CrashHandler)**: Omzeilt bekende crashes die ontstaan doordat BattlEye-bypass mechanismen bepaalde Steam-interfaces ongeïnitialiseerd laten. Wanneer een null-pointer dereference optreedt op specifieke offsets in `GTA5_Enhanced.exe`, wikkelt de exception handler het stackframe netjes af en hervat de executie zonder dat de game crasht naar de desktop.
- **Native Invoker**: Integreert met ScriptHookV om native engine-functies veilig uit te voeren via een gecoördineerde thread fiber.

---

## Vereisten en Bouwen

### Launcher bouwen (.NET SDK 8.0 vereist)

```bash
cd src/launcher
dotnet publish -c Release -r win-x64 --no-self-contained -p:PublishSingleFile=true
```

Het uitvoerbare bestand wordt gegenereerd in `src/launcher/bin/Release/net8.0-windows/win-x64/publish/GTAVCli.exe`.

### In-Game Menu bouwen (Clang / Zig C++ / MSVC)

Het in-game menu kan worden gecompileerd met de meegeleverde `build.bat` of via Clang met C++17 ondersteuning:

```bash
cd src/menu-dx12
build.bat
```

Dit resulteert in `EnhancedImGuiMenu.asi`, welke in de hoofdmap van GTA V Enhanced geplaatst kan worden.

---

## Gebruik

1. Start `GTAVCli.exe`.
2. Selecteer de gewenste editie (Enhanced Edition of Legacy Edition).
3. Kies welk mod menu voor de sessie geactiveerd moet worden.
4. De launcher past de bestandsextensies aan, start het spel op via Steam met de benodigde parameters, en monitort de status tot Story Mode actief is.
5. In-game kan het custom menu worden geopend met `INSERT` of `F11` (afhankelijk van de configuratie).
6. Na het afsluiten van de game herstelt de launcher direct alle mod-bestanden.

---

## Licentie en Disclaimer

Dit project is uitsluitend bedoeld voor offline single-player (Story Mode) gebruik. Het is niet bedoeld voor of compatibel met GTA Online. Gebruik van modificaties online is in strijd met de gebruiksvoorwaarden van Rockstar Games.
