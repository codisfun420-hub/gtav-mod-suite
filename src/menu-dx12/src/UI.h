#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "imgui/imgui.h"

struct ToastNotification {
    std::string message;
    ImVec4 color;
    DWORD expireTick;
    float maxDuration;
};

class UI {
public:
    static void InitStyle();
    static void Render();
    static void RenderTelemetryHUD();
    static void RenderMainMenu();
    static void RenderToasts();
    static void ShowToast(const std::string& message, ImVec4 color = ImVec4(0.2f, 0.9f, 0.4f, 1.0f));
    static bool HasActiveToasts();
    static void CycleTab(int delta);
    static void UpdateVirtualCursor(float deltaX, float deltaY, bool isClicking);

    static inline bool s_MenuOpen = false;
    static inline int s_ActiveTab = 0;
    static inline char s_SearchBuffer[128] = "";
    static inline char s_LicensePlateBuffer[16] = "ANTIGRAV";

    // Mod Shop State
    static inline int s_VehSubTab = 0; // 0 = Spawner, 1 = Mod Shop
    static inline int s_ModShopCategory = 0; // 0 = Carrosserie, 1 = Lakwerk & Neon, 2 = Wielen, 3 = Prestaties
    static inline float s_VehPrimaryColor[3] = { 0.0f, 0.85f, 1.0f };
    static inline float s_VehSecondaryColor[3] = { 0.1f, 0.1f, 0.1f };
    static inline float s_VehNeonColor[3] = { 0.0f, 0.85f, 1.0f };
    static inline float s_VehTyreSmokeColor[3] = { 1.0f, 0.1f, 0.1f };
    static inline bool s_VehNeonLeft = true;
    static inline bool s_VehNeonRight = true;
    static inline bool s_VehNeonFront = true;
    static inline bool s_VehNeonBack = true;
    static inline int s_VehXenonColor = 1; // Blue
    static inline int s_VehWindowTint = 1; // Pure Black
    static inline int s_VehWheelType = 0; // Sport
    static inline int s_VehWheelIndex = 0;
};
