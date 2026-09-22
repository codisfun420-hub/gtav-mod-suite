#include <windows.h>
#include <string>
#include "ScriptHookSDK.h"
#include "NativeQueue.h"
#include "Config.h"
#include "Logger.h"
#include "Cheats.h"
#include "UI.h"
#include "D3D12Hook.h"
#include "CrashHandler.h"

static HMODULE g_hModule = nullptr;
static std::string g_ModuleDir = "";

static void OnKeyboardMessage(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow) {
    if (!isUpNow && !wasDownBefore) { // Key down event
        if (key == (DWORD)g_Config.toggleKey || key == VK_F11) {
            UI::s_MenuOpen = !UI::s_MenuOpen;
            LOG_INFO("Menu toggled via ScriptHook keyboard handler (key: 0x%X) -> %s", key, UI::s_MenuOpen ? "OPEN" : "CLOSED");
        } else if (UI::s_MenuOpen && key == VK_ESCAPE) {
            UI::s_MenuOpen = false;
            LOG_INFO("Menu closed via ESC in ScriptHook keyboard handler.");
        }
    }
}

static void ScriptMain() {
    __try {
        LOG_INFO("ScriptHookV fiber script thread started for EnhancedImGuiMenu.");

        // Wait 500ms on first world spawn to allow engine systems, player ped, and scripts to fully stabilize
        ScriptHookSDK::scriptWait(500);

        UI::ShowToast("ScriptHookV Verbonden! Druk op [INSERT] of [F11]", ImVec4(0.2f, 1.0f, 0.4f, 1.0f));

        while (true) {
            // Redundant hotkey poll on script fiber thread
            static bool s_FiberKeyWasDown = false;
            bool isDown = ((GetAsyncKeyState(g_Config.toggleKey) & 0x8000) != 0) ||
                          ((GetAsyncKeyState(VK_F11) & 0x8000) != 0);
            if (isDown && !s_FiberKeyWasDown) {
                HWND fg = GetForegroundWindow();
                DWORD fgPid = 0;
                GetWindowThreadProcessId(fg, &fgPid);
                if (fgPid == GetCurrentProcessId()) {
                    UI::s_MenuOpen = !UI::s_MenuOpen;
                    LOG_INFO("Menu toggled via ScriptMain fiber -> %s", UI::s_MenuOpen ? "OPEN" : "CLOSED");
                }
            }
            s_FiberKeyWasDown = isDown;

            // 1. Process all queued native actions from the ImGui render thread safely
            NativeQueue::ProcessAll();

            // 2. Continuous cheat processing & telemetry update
            Cheats::OnTick();

            // 3. Suspend game camera & movement input while menu is open so player can click freely
            Cheats::SetInGameControlsDisabled(UI::s_MenuOpen);

            ScriptHookSDK::scriptWait(0);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        LOG_ERROR("Fatal exception 0x%08X caught in EnhancedImGuiMenu ScriptMain fiber!", GetExceptionCode());
    }
}

static DWORD WINAPI InitThread(LPVOID lpParam) {
    LOG_INFO("Initializing EnhancedImGuiMenu background setup thread for GTA V Enhanced Edition...");

    // 1. Hook DirectX 12 SwapChain & CommandQueue IMMEDIATELY (independent of ScriptHookV)
    if (!D3D12Hook::Init()) {
        LOG_WARN("Primary D3D12 Hook encountered warning, trying ScriptHookV present callback bridge...");
        if (ScriptHookSDK::presentCallbackRegister) {
            ScriptHookSDK::presentCallbackRegister([](void* swapChain) {
                // Secondary fallback bridge
            });
            LOG_INFO("Registered ScriptHookV present callback fallback bridge.");
        }
    }

    // Show initial welcome toast on-screen once DX12 renders
    UI::ShowToast("Enhanced Mod Menu Geladen! Druk op [INSERT] of [F11]", ImVec4(0.0f, 0.85f, 1.0f, 1.0f));

    // 2. Wait for ScriptHookV.dll to load in the game address space and resolve exports
    int attempts = 0;
    while (!ScriptHookSDK::Init(nullptr, g_ModuleDir.c_str()) && attempts < 100) {
        Sleep(150);
        attempts++;
    }

    if (ScriptHookSDK::scriptRegister) {
        ScriptHookSDK::scriptRegister(g_hModule, ScriptMain);
        LOG_INFO("ScriptHookV script fiber registered successfully.");
    } else {
        LOG_ERROR("Failed to register ScriptHookV script fiber after 100 attempts!");
    }

    if (ScriptHookSDK::keyboardHandlerRegister) {
        ScriptHookSDK::keyboardHandlerRegister(OnKeyboardMessage);
        LOG_INFO("ScriptHookV keyboard handler registered successfully.");
    }

    LOG_INFO("EnhancedImGuiMenu initialization complete. Press [INSERT] or [F11] in Story Mode to open.");
    return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            char processPath[MAX_PATH];
            GetModuleFileNameA(NULL, processPath, MAX_PATH);
            std::string proc(processPath);
            size_t pSlash = proc.find_last_of("\\/");
            std::string exeName = (pSlash != std::string::npos) ? proc.substr(pSlash + 1) : proc;

            // Strict process check: only attach and run if hosted by the actual GTA V game executable!
            if (_stricmp(exeName.c_str(), "GTA5_Enhanced.exe") != 0 && _stricmp(exeName.c_str(), "GTA5.exe") != 0) {
                return TRUE;
            }

            g_hModule = hModule;
            DisableThreadLibraryCalls(hModule);

            char moduleDir[MAX_PATH];
            GetModuleFileNameA(hModule, moduleDir, MAX_PATH);
            std::string dir(moduleDir);
            size_t lastSlash = dir.find_last_of("\\/");
            if (lastSlash != std::string::npos) {
                dir = dir.substr(0, lastSlash);
            }
            g_ModuleDir = dir;

            std::string logPath = dir + "\\EnhancedImGuiMenu.log";
            Logger::Init(logPath);
            Logger::OpenConsole();
            CrashHandler::Init();

            std::string iniPath = dir + "\\EnhancedImGuiMenu.ini";
            g_Config.Load(iniPath);

            CloseHandle(CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr));
            break;
        }

        case DLL_PROCESS_DETACH: {
            if (g_hModule) {
                LOG_INFO("EnhancedImGuiMenu detaching from process...");
                g_Config.Save(g_IniFilePath);
                CrashHandler::Shutdown();
                if (ScriptHookSDK::keyboardHandlerUnregister) {
                    ScriptHookSDK::keyboardHandlerUnregister(OnKeyboardMessage);
                }
                if (ScriptHookSDK::scriptUnregister) {
                    ScriptHookSDK::scriptUnregister(g_hModule);
                }
                D3D12Hook::Shutdown();
                Logger::Close();
            }
            break;
        }
    }
    return TRUE;
}
