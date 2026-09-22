#include "UI.h"
#include "Config.h"
#include "Cheats.h"
#include "NativeQueue.h"
#include "Logger.h"
#include "imgui/imgui.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>

static std::vector<ToastNotification> s_Toasts;
static std::mutex s_ToastMutex;

void UI::ShowToast(const std::string& message, ImVec4 color) {
    std::lock_guard<std::mutex> lock(s_ToastMutex);
    DWORD now = GetTickCount();
    // Keep max 4 concurrent toasts on screen
    if (s_Toasts.size() >= 4) {
        s_Toasts.erase(s_Toasts.begin());
    }
    s_Toasts.push_back({ message, color, now + 3200, 3.2f });
}

bool UI::HasActiveToasts() {
    std::lock_guard<std::mutex> lock(s_ToastMutex);
    return !s_Toasts.empty();
}

void UI::RenderToasts() {
    std::lock_guard<std::mutex> lock(s_ToastMutex);
    if (s_Toasts.empty()) return;

    DWORD now = GetTickCount();
    // Purge expired toasts
    s_Toasts.erase(
        std::remove_if(s_Toasts.begin(), s_Toasts.end(), [now](const ToastNotification& t) {
            return now >= t.expireTick;
        }),
        s_Toasts.end()
    );

    if (s_Toasts.empty()) return;

    ImGuiIO& io = ImGui::GetIO();
    float startY = 24.0f;
    float cardWidth = 360.0f;

    for (size_t i = 0; i < s_Toasts.size(); ++i) {
        const auto& toast = s_Toasts[i];
        float remaining = (float)(toast.expireTick - now) / 1000.0f;
        float alpha = 1.0f;
        if (remaining < 0.6f) {
            alpha = remaining / 0.6f;
        } else if ((toast.maxDuration - remaining) < 0.3f) {
            alpha = (toast.maxDuration - remaining) / 0.3f;
        }
        if (alpha < 0.05f) alpha = 0.05f;
        if (alpha > 1.0f) alpha = 1.0f;

        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - cardWidth - 24.0f, startY), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(cardWidth, 0), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.88f * alpha);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                 ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs;

        char windowId[32];
        snprintf(windowId, sizeof(windowId), "##Toast_%zu", i);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.2f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(toast.color.x, toast.color.y, toast.color.z, 0.85f * alpha));

        if (ImGui::Begin(windowId, nullptr, flags)) {
            ImGui::TextColored(ImVec4(toast.color.x, toast.color.y, toast.color.z, alpha), "[MOD MENU]");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.98f, alpha), "%s", toast.message.c_str());
            startY += ImGui::GetWindowHeight() + 10.0f;
        }
        ImGui::End();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }
}

static void ApplyGlassTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 12.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;

    style.WindowBorderSize = 1.2f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.WindowPadding = ImVec2(16.0f, 16.0f);
    style.FramePadding = ImVec2(10.0f, 7.0f);
    style.ItemSpacing = ImVec2(10.0f, 9.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);

    ImVec4* colors = style.Colors;
    ImVec4 acc = ImVec4(g_Config.accentColor[0], g_Config.accentColor[1], g_Config.accentColor[2], 1.0f);

    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.54f, 0.62f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.08f, 0.12f, g_Config.windowAlpha);
    colors[ImGuiCol_ChildBg] = ImVec4(0.11f, 0.13f, 0.18f, 0.60f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.09f, 0.10f, 0.15f, 0.95f);
    colors[ImGuiCol_Border] = ImVec4(acc.x, acc.y, acc.z, 0.35f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.16f, 0.22f, 0.70f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(acc.x * 0.35f, acc.y * 0.35f, acc.z * 0.35f, 0.45f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(acc.x * 0.50f, acc.y * 0.50f, acc.z * 0.50f, 0.60f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.10f, 0.15f, 0.90f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(acc.x * 0.25f, acc.y * 0.25f, acc.z * 0.25f, 0.90f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.08f, 0.12f, 0.60f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.11f, 0.13f, 0.18f, 0.80f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.09f, 0.12f, 0.40f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.28f, 0.38f, 0.70f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(acc.x * 0.5f, acc.y * 0.5f, acc.z * 0.5f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(acc.x, acc.y, acc.z, 0.90f);
    colors[ImGuiCol_CheckMark] = ImVec4(acc.x, acc.y, acc.z, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(acc.x, acc.y, acc.z, 0.85f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(acc.x, acc.y, acc.z, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.16f, 0.19f, 0.26f, 0.80f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(acc.x * 0.45f, acc.y * 0.45f, acc.z * 0.45f, 0.85f);
    colors[ImGuiCol_ButtonActive] = ImVec4(acc.x, acc.y, acc.z, 0.95f);
    colors[ImGuiCol_Header] = ImVec4(acc.x * 0.25f, acc.y * 0.25f, acc.z * 0.25f, 0.60f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(acc.x * 0.45f, acc.y * 0.45f, acc.z * 0.45f, 0.75f);
    colors[ImGuiCol_HeaderActive] = ImVec4(acc.x * 0.65f, acc.y * 0.65f, acc.z * 0.65f, 0.90f);
    colors[ImGuiCol_Separator] = ImVec4(acc.x, acc.y, acc.z, 0.25f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(acc.x, acc.y, acc.z, 0.60f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(acc.x, acc.y, acc.z, 0.85f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(acc.x, acc.y, acc.z, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(acc.x, acc.y, acc.z, 0.60f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(acc.x, acc.y, acc.z, 0.90f);
    colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.14f, 0.20f, 0.70f);
    colors[ImGuiCol_TabHovered] = ImVec4(acc.x * 0.45f, acc.y * 0.45f, acc.z * 0.45f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(acc.x * 0.35f, acc.y * 0.35f, acc.z * 0.35f, 0.95f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.11f, 0.16f, 0.60f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.16f, 0.22f, 0.80f);
}

void UI::InitStyle() {
    ApplyGlassTheme();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
}

void UI::CycleTab(int delta) {
    s_ActiveTab = (s_ActiveTab + delta + 7) % 7;
}

void UI::UpdateVirtualCursor(float deltaX, float deltaY, bool isClicking) {
    if (!s_MenuOpen) return;
    ImGuiIO& io = ImGui::GetIO();
    if (fabsf(deltaX) > 0.15f || fabsf(deltaY) > 0.15f) {
        float speed = g_Config.controllerCursorSpeed;
        float newX = io.MousePos.x + deltaX * speed;
        float newY = io.MousePos.y + deltaY * speed;
        if (newX < 0.0f) newX = 0.0f;
        if (newY < 0.0f) newY = 0.0f;
        if (newX > io.DisplaySize.x) newX = io.DisplaySize.x;
        if (newY > io.DisplaySize.y) newY = io.DisplaySize.y;
        io.MousePos = ImVec2(newX, newY);
    }
    io.MouseDown[0] = isClicking;
}

void UI::RenderTelemetryHUD() {
    if (!g_Config.showTelemetryHud) return;

    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags hudFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                ImGuiWindowFlags_NoNav;

    ImVec2 hudPos = ImVec2(io.DisplaySize.x - 280.0f, io.DisplaySize.y - 180.0f);
    ImGui::SetNextWindowPos(hudPos, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.75f);

    if (ImGui::Begin("##TelemetryHUD", nullptr, hudFlags)) {
        ImVec4 acc = ImVec4(g_Config.accentColor[0], g_Config.accentColor[1], g_Config.accentColor[2], 1.0f);

        const char* unit = g_Config.speedUnitKmh ? "KM/H" : "MPH";
        ImGui::TextColored(acc, "TELEMETRIE OVERLAY");
        ImGui::Separator();

        ImGui::Text("Snelheid: ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "%.1f %s", g_Telemetry.speed, unit);

        float healthFraction = (float)g_Telemetry.health / (float)(g_Telemetry.maxHealth > 0 ? g_Telemetry.maxHealth : 200);
        if (healthFraction < 0.0f) healthFraction = 0.0f;
        if (healthFraction > 1.0f) healthFraction = 1.0f;

        float armorFraction = (float)g_Telemetry.armor / 100.0f;
        if (armorFraction < 0.0f) armorFraction = 0.0f;
        if (armorFraction > 1.0f) armorFraction = 1.0f;

        char hBuf[32], aBuf[32];
        snprintf(hBuf, sizeof(hBuf), "HP: %d", g_Telemetry.health);
        snprintf(aBuf, sizeof(aBuf), "Armor: %d", g_Telemetry.armor);

        ImGui::ProgressBar(healthFraction, ImVec2(240, 14), hBuf);
        ImGui::ProgressBar(armorFraction, ImVec2(240, 14), aBuf);

        if (g_Telemetry.wantedLevel > 0) {
            std::string stars = "Wanted: ";
            for (int i = 0; i < g_Telemetry.wantedLevel; ++i) stars += "* ";
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%s", stars.c_str());
        } else {
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Wanted: Geen");
        }

        ImGui::TextDisabled("Pos: %.0f, %.0f, %.0f | FPS: %.0f", g_Telemetry.posX, g_Telemetry.posY, g_Telemetry.posZ, io.Framerate);
    }
    ImGui::End();
}

static bool CaseInsensitiveFind(const std::string& str, const std::string& query) {
    if (query.empty()) return true;
    auto it = std::search(
        str.begin(), str.end(),
        query.begin(), query.end(),
        [](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
    );
    return it != str.end();
}

void UI::RenderMainMenu() {
    if (!s_MenuOpen) return;

    ApplyGlassTheme();
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = g_Config.uiScale;

    ImGui::SetNextWindowSize(ImVec2(880.0f, 580.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f - 440.0f, io.DisplaySize.y * 0.5f - 290.0f), ImGuiCond_FirstUseEver);

    ImVec4 acc = ImVec4(g_Config.accentColor[0], g_Config.accentColor[1], g_Config.accentColor[2], 1.0f);

    if (ImGui::Begin("Grand Theft Auto V Enhanced - Mod Menu Overlay", &s_MenuOpen, ImGuiWindowFlags_NoCollapse)) {
        // Header
        ImGui::TextColored(acc, "ENHANCED IMGUI OVERLAY");
        ImGui::SameLine();
        ImGui::TextDisabled("| DirectX 12 Edition | Druk [INSERT] of [F11] om te openen / [ESC] om te sluiten");

        // Global Search Bar
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##GlobalSearch", "Zoek cheats, voertuigen, wapens, teleports, tuning... (bijv. 'god', 'adder', 'stamina', 'drift')", s_SearchBuffer, sizeof(s_SearchBuffer));
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        bool hasSearch = (s_SearchBuffer[0] != '\0');

        if (hasSearch) {
            // Search Mode Results
            ImGui::TextColored(acc, "Zoekresultaten voor: '%s'", s_SearchBuffer);
            ImGui::Separator();
            ImGui::BeginChild("SearchResults", ImVec2(0, 0), true);

            std::string q = s_SearchBuffer;

            // Player Search
            if (CaseInsensitiveFind("God Mode Invincible Onsterfelijk", q)) {
                if (ImGui::Checkbox("Player God Mode (Onsterfelijk)", &g_Config.playerGodMode)) {
                    g_Config.Save(g_IniFilePath);
                }
            }
            if (CaseInsensitiveFind("Heal Player Health Genezen Leven", q)) {
                if (ImGui::Button("Genees Speler Direct (100% HP)")) {
                    NativeQueue::Push([]() { Cheats::HealPlayer(); });
                }
            }
            if (CaseInsensitiveFind("Armor Schild Bodyarmor Pantser", q)) {
                if (ImGui::Button("Body Armor 100% Aanvullen")) {
                    NativeQueue::Push([]() { Cheats::AddArmor(); });
                }
            }
            if (CaseInsensitiveFind("Clean Wounds Blood Clothes Kleding Bloed Schoonmaken", q)) {
                if (ImGui::Button("Kleding & Schade Schoonmaken")) {
                    NativeQueue::Push([]() { Cheats::CleanPlayerDamage(); });
                }
            }
            if (CaseInsensitiveFind("Never Wanted Clear Police Politie Geen", q)) {
                if (ImGui::Checkbox("Never Wanted (Nooit gezocht)", &g_Config.playerNeverWanted)) {
                    g_Config.Save(g_IniFilePath);
                }
            }
            if (CaseInsensitiveFind("Stamina Uithoudingsvermogen Unlimited Moe", q)) {
                if (ImGui::Checkbox("Unlimited Stamina (Nooit moe)", &g_Config.playerUnlimitedStamina)) {
                    g_Config.Save(g_IniFilePath);
                }
            }
            if (CaseInsensitiveFind("Fast Swim Zwemmen Water Snel", q)) {
                if (ImGui::Checkbox("Fast Swim (Snel Zwemmen)", &g_Config.playerFastSwim)) {
                    g_Config.Save(g_IniFilePath);
                }
            }
            if (CaseInsensitiveFind("Special Ability Refill Meter", q)) {
                if (ImGui::Button("Special Ability Meter Vullen")) {
                    NativeQueue::Push([]() { Cheats::RefillSpecialAbility(); });
                }
            }
            if (CaseInsensitiveFind("Cash Money Geld Generator 100000 1000000 Miljoen", q)) {
                if (ImGui::Button("+$100,000 Cash##s")) {
                    NativeQueue::Push([]() { Cheats::AddCash(100000); });
                }
                ImGui::SameLine();
                if (ImGui::Button("+$1,000,000 Cash##s")) {
                    NativeQueue::Push([]() { Cheats::AddCash(1000000); });
                }
                ImGui::SameLine();
                if (ImGui::Button("Max $2.000.000.000##s")) {
                    NativeQueue::Push([]() { Cheats::SetMaxCash(); });
                }
            }
            if (CaseInsensitiveFind("Weapons All Wapens Ammo Geweren Guns", q)) {
                if (ImGui::Button("Geef Alle Wapens (Max Ammo)")) {
                    NativeQueue::Push([]() { Cheats::GiveAllWeapons(); });
                }
            }
            if (CaseInsensitiveFind("No Reload Unlimited Clip Magazijn Herladen", q)) {
                if (ImGui::Checkbox("No Reload (Oneindig Clip Magazijn)", &g_Config.weaponsNoReload)) {
                    g_Config.Save(g_IniFilePath);
                }
            }
            if (CaseInsensitiveFind("One Shot Kill Super Damage Schade", q)) {
                if (ImGui::Checkbox("One-Shot Kill (Extreme Kogelschade)", &g_Config.playerOneShotKill)) {
                    g_Config.Save(g_IniFilePath);
                }
            }
            if (CaseInsensitiveFind("Teleport Waypoint Map Marker Kaart", q)) {
                if (ImGui::Button("Teleporteer naar Waypoint")) {
                    NativeQueue::Push([]() { Cheats::TeleportToWaypoint(); });
                }
            }
            if (CaseInsensitiveFind("Explosive Ammo Melee Vuur Bullets", q)) {
                if (ImGui::Checkbox("Explosieve Kogels##s", &g_Config.weaponsExplosiveAmmo)) g_Config.Save(g_IniFilePath);
                ImGui::SameLine();
                if (ImGui::Checkbox("Brandende Kogels##s", &g_Config.weaponsFireBullets)) g_Config.Save(g_IniFilePath);
                ImGui::SameLine();
                if (ImGui::Checkbox("Explosieve Vuisten##s", &g_Config.weaponsExplosiveMelee)) g_Config.Save(g_IniFilePath);
            }
            if (CaseInsensitiveFind("Noclip Freecam Vliegen Muren Ghost Geest", q)) {
                if (ImGui::Checkbox("Noclip / Freecam (Vlieg door muren)##s", &g_Config.playerNoclip)) {
                    g_Config.Save(g_IniFilePath);
                }
                if (g_Config.playerNoclip) {
                    if (ImGui::SliderFloat("Vliegsnelheid (Noclip)##s", &g_Config.noclipSpeed, 0.2f, 5.0f, "%.1fx")) {
                        g_Config.Save(g_IniFilePath);
                    }
                }
            }
            if (CaseInsensitiveFind("Gravity Gun Physics Zwaartekracht Wapen Schieten", q)) {
                if (ImGui::Checkbox("Gravity Gun (Physics Manipulator)##s", &g_Config.weaponsGravityGun)) {
                    g_Config.Save(g_IniFilePath);
                }
            }
            if (CaseInsensitiveFind("Drift Grip Banden Glijden", q)) {
                if (ImGui::Checkbox("Drift Mode (Verminderde grip)", &g_Config.vehicleDriftMode)) {
                    g_Config.Save(g_IniFilePath);
                }
            }
            if (CaseInsensitiveFind("Torque Power Vermogen Snelheid Boost", q)) {
                if (ImGui::Checkbox("Torque & Vermogen Boost##s", &g_Config.vehicleTorqueBoost)) {
                    g_Config.Save(g_IniFilePath);
                }
                if (g_Config.vehicleTorqueBoost) {
                    if (ImGui::SliderFloat("Vermogen Multiplier##s", &g_Config.vehicleTorqueMultiplier, 1.0f, 15.0f, "%.1fx")) {
                        g_Config.Save(g_IniFilePath);
                    }
                }
            }
            if (CaseInsensitiveFind("Rainbow RGB Paint Kleur Auto", q)) {
                if (ImGui::Checkbox("Rainbow RGB Car Paint", &g_Config.vehicleRainbowPaint)) {
                    g_Config.Save(g_IniFilePath);
                }
            }
            if (CaseInsensitiveFind("Auto Flip Rechtop Roll", q)) {
                if (ImGui::Checkbox("Auto-Flip Upright (Rechtop zetten)", &g_Config.vehicleAutoFlip)) {
                    g_Config.Save(g_IniFilePath);
                }
            }
            if (CaseInsensitiveFind("Repair Clean Tuning Repareren Schoonmaken", q)) {
                if (ImGui::Button("Repareer & Reinig Voertuig##s")) {
                    NativeQueue::Push([]() { Cheats::RepairVehicle(); });
                }
                ImGui::SameLine();
                if (ImGui::Button("Max Performance Tuning##s")) {
                    NativeQueue::Push([]() { Cheats::MaxTuneVehicle(); });
                }
            }
            if (CaseInsensitiveFind("Blackout Donker Licht Stad Lights", q)) {
                if (ImGui::Checkbox("Blackout Mode (Stadslichten uit)", &g_Config.worldBlackout)) {
                    bool b = g_Config.worldBlackout;
                    NativeQueue::Push([b]() { Cheats::SetBlackout(b); });
                    g_Config.Save(g_IniFilePath);
                }
            }
            if (CaseInsensitiveFind("Matrix Slow Motion Bullet Time Vertragen", q)) {
                if (ImGui::SliderFloat("Matrix Slow-Motion##s", &g_Config.worldMatrixTimeScale, 0.1f, 1.0f, "%.2fx")) {
                    float s = g_Config.worldMatrixTimeScale;
                    NativeQueue::Push([s]() { Cheats::SetTimeScale(s); });
                    g_Config.Save(g_IniFilePath);
                }
            }

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Overeenkomende Voertuigen:");
            for (const auto& v : g_VehicleList) {
                if (CaseInsensitiveFind(v.displayName, q) || CaseInsensitiveFind(v.modelName, q) || CaseInsensitiveFind(v.category, q)) {
                    char btnLabel[128];
                    snprintf(btnLabel, sizeof(btnLabel), "Spawn %s (%s)##search", v.displayName, v.category);
                    if (ImGui::Button(btnLabel)) {
                        const char* m = v.modelName;
                        NativeQueue::Push([m]() { Cheats::SpawnVehicle(m); });
                    }
                }
            }

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Overeenkomende Teleport Locaties:");
            for (const auto& loc : g_TeleportList) {
                if (CaseInsensitiveFind(loc.name, q) || CaseInsensitiveFind(loc.category, q)) {
                    char btnLabel[128];
                    snprintf(btnLabel, sizeof(btnLabel), "Teleport: %s [%s]##search", loc.name, loc.category);
                    if (ImGui::Button(btnLabel)) {
                        float x = loc.x, y = loc.y, z = loc.z;
                        NativeQueue::Push([x, y, z]() { Cheats::TeleportToCoords(x, y, z); });
                    }
                }
            }

            ImGui::EndChild();
        } else {
            // Standard Tab Layout
            // Left-Side Sidebar Navigation
            ImGui::BeginChild("Sidebar", ImVec2(180, 0), true);

            const char* tabs[] = { "[P] Speler", "[V] Voertuigen", "[H] Handling & Drift", "[W] Wapens", "[T] Teleport", "[E] Wereld", "[S] Instellingen" };
            for (int i = 0; i < 7; ++i) {
                bool isSelected = (s_ActiveTab == i);
                if (isSelected) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(acc.x * 0.45f, acc.y * 0.45f, acc.z * 0.45f, 0.95f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                }

                if (ImGui::Button(tabs[i], ImVec2(-1, 38))) {
                    s_ActiveTab = i;
                }

                if (isSelected) {
                    ImGui::PopStyleColor(2);
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextDisabled("Status Badges:");
            if (g_Config.playerGodMode) ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "[GOD MODE]");
            if (g_Config.playerNeverWanted) ImGui::TextColored(ImVec4(0.2f, 0.9f, 1.0f, 1.0f), "[NO WANTED]");
            if (g_Config.playerNoclip) ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.9f, 1.0f), "[NOCLIP]");
            if (g_Config.weaponsInfiniteAmmo) ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "[INF AMMO]");
            if (g_Config.weaponsNoReload) ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.3f, 1.0f), "[NO RELOAD]");
            if (g_Config.weaponsGravityGun) ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.2f, 1.0f), "[GRAV GUN]");
            if (g_Config.vehicleGodMode) ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.8f, 1.0f), "[VEH GOD]");
            if (g_Config.vehicleDriftMode) ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "[DRIFT]");
            if (g_Config.vehicleTorqueBoost) ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "[TORQUE]");
            if (g_Config.worldBlackout) ImGui::TextColored(ImVec4(0.8f, 0.4f, 1.0f, 1.0f), "[BLACKOUT]");

            ImGui::EndChild();

            ImGui::SameLine();

            // Right-Side Tab Content Pane
            ImGui::BeginChild("TabContent", ImVec2(0, 0), true);

            switch (s_ActiveTab) {
                // TAB 0: SPELER
                case 0: {
                    ImGui::TextColored(acc, "SPELER MODIFICATIES & VAARDIGHEDEN");
                    ImGui::Separator();

                    if (ImGui::Checkbox("God Mode (Onsterfelijk)", &g_Config.playerGodMode)) {
                        g_Config.Save(g_IniFilePath);
                    }
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Never Wanted (Nooit Gezocht)", &g_Config.playerNeverWanted)) {
                        g_Config.Save(g_IniFilePath);
                    }

                    ImGui::Spacing();
                    if (ImGui::Button("Genees Speler (100% HP)", ImVec2(200, 32))) {
                        NativeQueue::Push([]() { Cheats::HealPlayer(); });
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Body Armor Aanvullen", ImVec2(180, 32))) {
                        NativeQueue::Push([]() { Cheats::AddArmor(); });
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Kleding Reinigen", ImVec2(160, 32))) {
                        NativeQueue::Push([]() { Cheats::CleanPlayerDamage(); });
                    }

                    ImGui::Spacing();
                    ImGui::Text("Wanted Level Aanpassen:");
                    if (ImGui::Button("Wis Wanted Level", ImVec2(160, 28))) {
                        NativeQueue::Push([]() { Cheats::ClearWantedLevel(); });
                    }
                    ImGui::SameLine();
                    for (int s = 1; s <= 5; ++s) {
                        char starLabel[16];
                        snprintf(starLabel, sizeof(starLabel), "%d*", s);
                        if (ImGui::Button(starLabel, ImVec2(34, 28))) {
                            NativeQueue::Push([s]() { Cheats::SetWantedLevel(s); });
                        }
                        if (s < 5) ImGui::SameLine();
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Text("Vrij Rondvliegen & Navigatie:");
                    if (ImGui::Checkbox("Noclip / Freecam (Vlieg door muren met WASD/Spatie/Ctrl)", &g_Config.playerNoclip)) {
                        g_Config.Save(g_IniFilePath);
                    }
                    if (g_Config.playerNoclip) {
                        if (ImGui::SliderFloat("Vliegsnelheid (Noclip)", &g_Config.noclipSpeed, 0.2f, 5.0f, "%.1fx Snelheid")) {
                            g_Config.Save(g_IniFilePath);
                        }
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Text("Fysica, Sprint & Krachten:");
                    if (ImGui::Checkbox("Super Jump", &g_Config.playerSuperJump)) g_Config.Save(g_IniFilePath);
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Fast Sprint Multiplier", &g_Config.playerFastSprint)) g_Config.Save(g_IniFilePath);
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Fast Swim (Zwemmen)", &g_Config.playerFastSwim)) g_Config.Save(g_IniFilePath);

                    if (ImGui::Checkbox("Unlimited Stamina (Nooit moe)", &g_Config.playerUnlimitedStamina)) g_Config.Save(g_IniFilePath);
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Invisibility (Onzichtbaar)", &g_Config.playerInvisibility)) g_Config.Save(g_IniFilePath);
                    ImGui::SameLine();
                    if (ImGui::Checkbox("One-Shot Kill", &g_Config.playerOneShotKill)) g_Config.Save(g_IniFilePath);

                    ImGui::Spacing();
                    if (ImGui::Button("Special Ability Meter Vullen", ImVec2(220, 30))) {
                        NativeQueue::Push([]() { Cheats::RefillSpecialAbility(); });
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Ragdoll Push", ImVec2(140, 30))) {
                        NativeQueue::Push([]() { Cheats::RagdollPlayer(); });
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Zelfmoord / Respawn", ImVec2(160, 30))) {
                        NativeQueue::Push([]() { Cheats::SuicidePlayer(); });
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Text("Cash Generator (Story Mode Bankrekening):");
                    if (ImGui::Button("+$100,000", ImVec2(100, 30))) {
                        NativeQueue::Push([]() { Cheats::AddCash(100000); });
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("+$1,000,000", ImVec2(110, 30))) {
                        NativeQueue::Push([]() { Cheats::AddCash(1000000); });
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("+$10,000,000", ImVec2(110, 30))) {
                        NativeQueue::Push([]() { Cheats::AddCash(10000000); });
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("+$50,000,000", ImVec2(110, 30))) {
                        NativeQueue::Push([]() { Cheats::AddCash(50000000); });
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Max $2.000.000.000", ImVec2(150, 30))) {
                        NativeQueue::Push([]() { Cheats::SetMaxCash(); });
                    }
                    break;
                }

                // TAB 1: VOERTUIGEN
                case 1: {
                    ImGui::TextColored(acc, "VOERTUIG CONTROLS, SPAWNER & LOS SANTOS CUSTOMS");
                    ImGui::Separator();

                    // Sub-navigation: Spawner vs Mod Shop
                    ImGui::PushStyleColor(ImGuiCol_Button, s_VehSubTab == 0 ? ImVec4(acc.x * 0.4f, acc.y * 0.4f, acc.z * 0.4f, 0.9f) : ImVec4(0.15f, 0.18f, 0.25f, 0.7f));
                    if (ImGui::Button("🚀 Spawner & Snelle Cheats", ImVec2(220, 32))) s_VehSubTab = 0;
                    ImGui::PopStyleColor();

                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Button, s_VehSubTab == 1 ? ImVec4(acc.x * 0.4f, acc.y * 0.4f, acc.z * 0.4f, 0.9f) : ImVec4(0.15f, 0.18f, 0.25f, 0.7f));
                    if (ImGui::Button("🔧 Los Santos Customs (Mod Shop)", ImVec2(250, 32))) s_VehSubTab = 1;
                    ImGui::PopStyleColor();
                    ImGui::Separator();

                    // SUB-TAB 0: SPAWNER & SNELLE CHEATS
                    if (s_VehSubTab == 0) {
                        if (ImGui::Checkbox("Vehicle God Mode (Onverwoestbaar)", &g_Config.vehicleGodMode)) g_Config.Save(g_IniFilePath);
                        ImGui::SameLine();
                        if (ImGui::Checkbox("Veiligheidsgordel (Seatbelt)", &g_Config.vehicleSeatbelt)) g_Config.Save(g_IniFilePath);
                        ImGui::SameLine();
                        if (ImGui::Checkbox("Auto-Flip Upright", &g_Config.vehicleAutoFlip)) g_Config.Save(g_IniFilePath);

                        if (ImGui::Checkbox("Drift Mode (Verminderde Grip)", &g_Config.vehicleDriftMode)) g_Config.Save(g_IniFilePath);
                        ImGui::SameLine();
                        if (ImGui::Checkbox("Rainbow RGB Car Paint", &g_Config.vehicleRainbowPaint)) g_Config.Save(g_IniFilePath);
                        ImGui::SameLine();
                        if (ImGui::Checkbox("Spawn Direct Max-Tuned", &g_Config.spawnMaxTuned)) g_Config.Save(g_IniFilePath);

                        ImGui::Spacing();
                        if (ImGui::Button("Repareer & Reinig Voertuig", ImVec2(200, 32))) {
                            NativeQueue::Push([]() { Cheats::RepairVehicle(); });
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Max Performance Tuning", ImVec2(190, 32))) {
                            NativeQueue::Push([]() { Cheats::MaxTuneVehicle(); });
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Directe Noodstop (0 m/s)", ImVec2(180, 32))) {
                            NativeQueue::Push([]() { Cheats::InstantBrake(); });
                        }

                        ImGui::Spacing();
                        if (ImGui::Button("Rocket Boost (+30 m/s)", ImVec2(180, 30))) {
                            NativeQueue::Push([]() { Cheats::BoostVehicle(30.0f); });
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Super Rocket Boost (+60 m/s)", ImVec2(210, 30))) {
                            NativeQueue::Push([]() { Cheats::BoostVehicle(60.0f); });
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Zet Voertuig Rechtop", ImVec2(160, 30))) {
                            NativeQueue::Push([]() { Cheats::AutoFlipVehicle(); });
                        }

                        ImGui::Spacing();
                        ImGui::SetNextItemWidth(160);
                        ImGui::InputText("##Kenteken", s_LicensePlateBuffer, sizeof(s_LicensePlateBuffer));
                        ImGui::SameLine();
                        if (ImGui::Button("Stel Kentekenplaat In")) {
                            std::string p = s_LicensePlateBuffer;
                            NativeQueue::Push([p]() { Cheats::SetLicensePlate(p.c_str()); });
                        }

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Text("Gecategoriseerde Voertuig Spawner:");

                        static const char* catFilters[] = { "All", "Supers", "Sports", "Muscle", "Off-Road", "Helicopters", "Planes", "Motorcycles", "Emergency" };
                        static int currentCat = 0;
                        ImGui::Combo("Filter Categorie", &currentCat, catFilters, IM_ARRAYSIZE(catFilters));

                        ImGui::BeginChild("VehicleGrid", ImVec2(0, 0), true);
                        int col = 0;
                        for (const auto& v : g_VehicleList) {
                            if (currentCat != 0 && strcmp(v.category, catFilters[currentCat]) != 0) continue;

                            char vLabel[128];
                            snprintf(vLabel, sizeof(vLabel), "%s##vsp", v.displayName);
                            if (ImGui::Button(vLabel, ImVec2(180, 32))) {
                                const char* model = v.modelName;
                                NativeQueue::Push([model]() { Cheats::SpawnVehicle(model); });
                            }
                            col++;
                            if (col % 3 != 0) ImGui::SameLine();
                        }
                        ImGui::EndChild();
                    }
                    // SUB-TAB 1: LOS SANTOS CUSTOMS / MOD SHOP
                    else {
                        if (!g_Telemetry.inVehicle) {
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "⚠️ Stap eerst in een voertuig om de Los Santos Customs Mod Shop te gebruiken!");
                            ImGui::TextDisabled("Je moet in een auto of motor zitten om onderdelen, verlichting en lakkleuren live aan te passen.");
                        } else {
                            // Mod Shop Category Pills
                            ImGui::PushStyleColor(ImGuiCol_Button, s_ModShopCategory == 0 ? ImVec4(acc.x * 0.4f, acc.y * 0.4f, acc.z * 0.4f, 0.9f) : ImVec4(0.15f, 0.18f, 0.25f, 0.7f));
                            if (ImGui::Button("🚗 Carrosserie", ImVec2(130, 28))) s_ModShopCategory = 0;
                            ImGui::PopStyleColor();

                            ImGui::SameLine();
                            ImGui::PushStyleColor(ImGuiCol_Button, s_ModShopCategory == 1 ? ImVec4(acc.x * 0.4f, acc.y * 0.4f, acc.z * 0.4f, 0.9f) : ImVec4(0.15f, 0.18f, 0.25f, 0.7f));
                            if (ImGui::Button("🎨 Lakwerk & Neon", ImVec2(150, 28))) s_ModShopCategory = 1;
                            ImGui::PopStyleColor();

                            ImGui::SameLine();
                            ImGui::PushStyleColor(ImGuiCol_Button, s_ModShopCategory == 2 ? ImVec4(acc.x * 0.4f, acc.y * 0.4f, acc.z * 0.4f, 0.9f) : ImVec4(0.15f, 0.18f, 0.25f, 0.7f));
                            if (ImGui::Button("🛞 Wielen & Banden", ImVec2(150, 28))) s_ModShopCategory = 2;
                            ImGui::PopStyleColor();

                            ImGui::SameLine();
                            ImGui::PushStyleColor(ImGuiCol_Button, s_ModShopCategory == 3 ? ImVec4(acc.x * 0.4f, acc.y * 0.4f, acc.z * 0.4f, 0.9f) : ImVec4(0.15f, 0.18f, 0.25f, 0.7f));
                            if (ImGui::Button("⚡ Prestaties", ImVec2(130, 28))) s_ModShopCategory = 3;
                            ImGui::PopStyleColor();
                            ImGui::Separator();

                            auto RenderModStepper = [](const char* name, int modType) {
                                int count = Cheats::GetVehicleModCount(modType);
                                int cur = Cheats::GetVehicleMod(modType);
                                ImGui::PushID(modType);
                                if (count <= 0) {
                                    ImGui::TextDisabled("%-20s: Geen onderdelen beschikbaar", name);
                                } else {
                                    char prevBtn[32], nextBtn[32], label[64];
                                    snprintf(prevBtn, sizeof(prevBtn), "<##%d", modType);
                                    snprintf(nextBtn, sizeof(nextBtn), ">##%d", modType);
                                    snprintf(label, sizeof(label), "%-18s: %d / %d", name, cur + 1, count);
                                    if (ImGui::Button(prevBtn, ImVec2(32, 26))) {
                                        int nextIdx = cur > -1 ? cur - 1 : count - 1;
                                        NativeQueue::Push([modType, nextIdx]() { Cheats::SetVehicleMod(modType, nextIdx); });
                                    }
                                    ImGui::SameLine();
                                    if (ImGui::Button(nextBtn, ImVec2(32, 26))) {
                                        int nextIdx = cur < count - 1 ? cur + 1 : -1;
                                        NativeQueue::Push([modType, nextIdx]() { Cheats::SetVehicleMod(modType, nextIdx); });
                                    }
                                    ImGui::SameLine();
                                    ImGui::Text("%s", label);
                                    ImGui::SameLine();
                                    if (ImGui::SmallButton("Standaard (Stock)")) {
                                        NativeQueue::Push([modType]() { Cheats::SetVehicleMod(modType, -1); });
                                    }
                                }
                                ImGui::PopID();
                            };

                            // Category 0: Carrosserie
                            if (s_ModShopCategory == 0) {
                                ImGui::Text("Pas spoilers, bumpers, motorkappen en carrosseriedelen aan:");
                                RenderModStepper("Spoiler", 0);
                                RenderModStepper("Voorbumper", 1);
                                RenderModStepper("Achterbumper", 2);
                                RenderModStepper("Sideskirts", 3);
                                RenderModStepper("Uitlaat", 4);
                                RenderModStepper("Rolkooi / Frame", 5);
                                RenderModStepper("Grille", 6);
                                RenderModStepper("Motorkap", 7);
                                RenderModStepper("Spatborden Links", 8);
                                RenderModStepper("Spatborden Rechts", 9);
                                RenderModStepper("Dak", 10);
                                RenderModStepper("Livery (Bestickering)", 48);
                            }
                            // Category 1: Lakwerk & Neon
                            else if (s_ModShopCategory == 1) {
                                ImGui::Text("Aangepaste RGB Carrosserie & Neon Verlichting:");
                                if (ImGui::ColorEdit3("Primaire Lak (RGB)", s_VehPrimaryColor)) {
                                    float pr = s_VehPrimaryColor[0], pg = s_VehPrimaryColor[1], pb = s_VehPrimaryColor[2];
                                    float sr = s_VehSecondaryColor[0], sg = s_VehSecondaryColor[1], sb = s_VehSecondaryColor[2];
                                    NativeQueue::Push([pr, pg, pb, sr, sg, sb]() { Cheats::SetVehicleCustomColors(pr, pg, pb, sr, sg, sb); });
                                }

                                if (ImGui::ColorEdit3("Secundaire Lak (RGB)", s_VehSecondaryColor)) {
                                    float pr = s_VehPrimaryColor[0], pg = s_VehPrimaryColor[1], pb = s_VehPrimaryColor[2];
                                    float sr = s_VehSecondaryColor[0], sg = s_VehSecondaryColor[1], sb = s_VehSecondaryColor[2];
                                    NativeQueue::Push([pr, pg, pb, sr, sg, sb]() { Cheats::SetVehicleCustomColors(pr, pg, pb, sr, sg, sb); });
                                }

                                ImGui::Spacing();
                                ImGui::Separator();
                                ImGui::Text("4-Zijdige Neon Ondergloed:");
                                bool neonChanged = false;
                                neonChanged |= ImGui::Checkbox("Links##nl", &s_VehNeonLeft);
                                ImGui::SameLine();
                                neonChanged |= ImGui::Checkbox("Rechts##nr", &s_VehNeonRight);
                                ImGui::SameLine();
                                neonChanged |= ImGui::Checkbox("Voor##nf", &s_VehNeonFront);
                                ImGui::SameLine();
                                neonChanged |= ImGui::Checkbox("Achter##nb", &s_VehNeonBack);
                                neonChanged |= ImGui::ColorEdit3("Neon Kleur (RGB)", s_VehNeonColor);
                                if (neonChanged) {
                                    bool nl = s_VehNeonLeft, nr = s_VehNeonRight, nf = s_VehNeonFront, nb = s_VehNeonBack;
                                    float nr_ = s_VehNeonColor[0], ng_ = s_VehNeonColor[1], nb_ = s_VehNeonColor[2];
                                    NativeQueue::Push([nl, nr, nf, nb, nr_, ng_, nb_]() {
                                        Cheats::SetVehicleNeonState(nl, nr, nf, nb, nr_, ng_, nb_);
                                    });
                                }

                                ImGui::Spacing();
                                ImGui::Separator();
                                static const char* xenonNames[] = {
                                    "0 - Wit (Stock)", "1 - Blauw", "2 - Elektrisch Blauw", "3 - Mintgroen",
                                    "4 - Limoengroen", "5 - Geel", "6 - Goudgeel", "7 - Oranje",
                                    "8 - Rood", "9 - Pony Roze", "10 - Felroze", "11 - Paars", "12 - Blacklight"
                                };
                                if (ImGui::Combo("Xenon Koplampkleur", &s_VehXenonColor, xenonNames, IM_ARRAYSIZE(xenonNames))) {
                                    int col = s_VehXenonColor;
                                    NativeQueue::Push([col]() { Cheats::SetVehicleXenonColor(col); });
                                }

                                static const char* tintNames[] = {
                                    "0 - Geen / Stock", "1 - Pure Black (Limo)", "2 - Dark Smoke",
                                    "3 - Light Smoke", "4 - Stock Tint", "5 - Limo Black"
                                };
                                if (ImGui::Combo("Ramen Tinten (Privacy Glass)", &s_VehWindowTint, tintNames, IM_ARRAYSIZE(tintNames))) {
                                    int tint = s_VehWindowTint;
                                    NativeQueue::Push([tint]() { Cheats::SetVehicleWindowTint(tint); });
                                }
                            }
                            // Category 2: Wielen & Banden
                            else if (s_ModShopCategory == 2) {
                                ImGui::Text("Wieltype, Velgen & Burnout Rook:");
                                static const char* wheelTypes[] = {
                                    "0 - Sport", "1 - Muscle", "2 - Lowrider", "3 - SUV",
                                    "4 - Offroad", "5 - Tuning", "6 - Bike", "7 - High-End",
                                    "8 - Benny's Original", "9 - Benny's Bespoke", "10 - Open Wheel", "11 - Street"
                                };
                                if (ImGui::Combo("Wielcategorie", &s_VehWheelType, wheelTypes, IM_ARRAYSIZE(wheelTypes))) {
                                    int wt = s_VehWheelType;
                                    int wi = s_VehWheelIndex;
                                    NativeQueue::Push([wt, wi]() { Cheats::SetVehicleWheelType(wt, wi); });
                                }

                                RenderModStepper("Velg Model", 23);

                                if (ImGui::ColorEdit3("Burnout Bandenrook Kleur", s_VehTyreSmokeColor)) {
                                    float r = s_VehTyreSmokeColor[0], g = s_VehTyreSmokeColor[1], b = s_VehTyreSmokeColor[2];
                                    NativeQueue::Push([r, g, b]() { Cheats::SetVehicleTyreSmokeColor(r, g, b); });
                                }
                            }
                            // Category 3: Prestaties
                            else if (s_ModShopCategory == 3) {
                                ImGui::Text("Prestatie-upgrades & Engine Tuning:");
                                RenderModStepper("Motor Tuning (EMS)", 11);
                                RenderModStepper("Remmen", 12);
                                RenderModStepper("Transmissie", 13);
                                RenderModStepper("Ophanging / Vering", 15);
                                RenderModStepper("Armor (Bepantsering)", 16);

                                ImGui::Spacing();
                                if (ImGui::Button("Schakel Turbo Tuning In", ImVec2(220, 30))) {
                                    NativeQueue::Push([]() { Cheats::ToggleVehicleExtraMod(18, true); });
                                }
                                ImGui::SameLine();
                                if (ImGui::Button("Alles Maximaal Tunen", ImVec2(200, 30))) {
                                    NativeQueue::Push([]() { Cheats::MaxTuneVehicle(); });
                                }
                            }
                        }
                    }
                    break;
                }

                // TAB 2: HANDLING & DRIFT
                case 2: {
                    ImGui::TextColored(acc, "HANDLING, DRIFT & VOERTUIGFYSICA");
                    ImGui::Separator();

                    if (ImGui::Checkbox("Drift Mode (Grip Reductie)", &g_Config.vehicleDriftMode)) {
                        g_Config.Save(g_IniFilePath);
                    }
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Torque & Vermogen Boost", &g_Config.vehicleTorqueBoost)) {
                        g_Config.Save(g_IniFilePath);
                    }

                    if (g_Config.vehicleTorqueBoost) {
                        if (ImGui::SliderFloat("Vermogens Multiplier", &g_Config.vehicleTorqueMultiplier, 1.0f, 15.0f, "%.1fx Vermogen")) {
                            g_Config.Save(g_IniFilePath);
                        }
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Text("Fysica, Assistentie & Stijl:");
                    if (ImGui::Checkbox("Auto-Flip Upright (Herstel bij koprol)", &g_Config.vehicleAutoFlip)) g_Config.Save(g_IniFilePath);
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Veiligheidsgordel (Niet door voorruit)", &g_Config.vehicleSeatbelt)) g_Config.Save(g_IniFilePath);
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Regenboog RGB Lakwerk", &g_Config.vehicleRainbowPaint)) g_Config.Save(g_IniFilePath);

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Text("Directe Voertuigacties:");
                    if (ImGui::Button("Instant Turbo Boost (+50 km/u)", ImVec2(240, 32))) {
                        NativeQueue::Push([]() { Cheats::BoostVehicle(40.0f); });
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Instant Noodstop (0 km/u)", ImVec2(200, 32))) {
                        NativeQueue::Push([]() { Cheats::InstantBrake(); });
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Zet Auto Rechtop", ImVec2(180, 32))) {
                        NativeQueue::Push([]() { Cheats::AutoFlipVehicle(); });
                    }
                    break;
                }

                // TAB 3: WAPENS
                case 3: {
                    ImGui::TextColored(acc, "WAPENUITRUSTING & MUNITIEMODIFICATIES");
                    ImGui::Separator();

                    if (ImGui::Button("Geef Alle Wapens (Max Ammo)", ImVec2(280, 36))) {
                        NativeQueue::Push([]() { Cheats::GiveAllWeapons(); });
                    }

                    ImGui::Spacing();
                    ImGui::Text("Wapens per categorie uitdelen:");
                    if (ImGui::Button("Pistolen", ImVec2(120, 28))) NativeQueue::Push([]() { Cheats::GiveHandguns(); });
                    ImGui::SameLine();
                    if (ImGui::Button("Rifles & SMG's", ImVec2(140, 28))) NativeQueue::Push([]() { Cheats::GiveRiflesAndSMGs(); });
                    ImGui::SameLine();
                    if (ImGui::Button("Shotguns & Snipers", ImVec2(150, 28))) NativeQueue::Push([]() { Cheats::GiveShotgunsAndSnipers(); });
                    ImGui::SameLine();
                    if (ImGui::Button("Zware Wapens (RPG/Minigun)", ImVec2(180, 28))) NativeQueue::Push([]() { Cheats::GiveHeavyWeapons(); });
                    ImGui::SameLine();
                    if (ImGui::Button("Melee", ImVec2(80, 28))) NativeQueue::Push([]() { Cheats::GiveMeleeWeapons(); });

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Text("Munitie & Kogel Modifiers:");

                    if (ImGui::Checkbox("Oneindig Munitie (Infinite Ammo)", &g_Config.weaponsInfiniteAmmo)) g_Config.Save(g_IniFilePath);
                    ImGui::SameLine();
                    if (ImGui::Checkbox("No Reload (Nooit Herladen)", &g_Config.weaponsNoReload)) g_Config.Save(g_IniFilePath);
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Rapid Fire Multiplier", &g_Config.weaponsRapidFire)) g_Config.Save(g_IniFilePath);

                    if (ImGui::Checkbox("Explosieve Kogels", &g_Config.weaponsExplosiveAmmo)) g_Config.Save(g_IniFilePath);
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Brandende / Incendiary Kogels", &g_Config.weaponsFireBullets)) g_Config.Save(g_IniFilePath);
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Explosieve Vuistslagen", &g_Config.weaponsExplosiveMelee)) g_Config.Save(g_IniFilePath);

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Text("Speciale Fysica-Wapens:");
                    if (ImGui::Checkbox("Gravity Gun (R-Muis = vastpakken, L-Muis = wegschieten)", &g_Config.weaponsGravityGun)) {
                        g_Config.Save(g_IniFilePath);
                    }
                    break;
                }

                // TAB 4: TELEPORT
                case 4: {
                    ImGui::TextColored(acc, "TELEPORTATIE & LOCATIES");
                    ImGui::Separator();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(acc.x * 0.45f, acc.y * 0.45f, acc.z * 0.45f, 0.95f));
                    if (ImGui::Button("Teleporteer naar Map Waypoint (Punt op de kaart)", ImVec2(-1, 38))) {
                        NativeQueue::Push([]() { Cheats::TeleportToWaypoint(); });
                    }
                    ImGui::PopStyleColor();

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Text("Bekende Locaties & Bezienswaardigheden:");

                    ImGui::BeginChild("TeleportGrid", ImVec2(0, 0), true);
                    for (const auto& loc : g_TeleportList) {
                        char locLabel[128];
                        snprintf(locLabel, sizeof(locLabel), "%s (%s)", loc.name, loc.category);
                        if (ImGui::Button(locLabel, ImVec2(-1, 30))) {
                            float x = loc.x, y = loc.y, z = loc.z;
                            NativeQueue::Push([x, y, z]() { Cheats::TeleportToCoords(x, y, z); });
                        }
                    }
                    ImGui::EndChild();
                    break;
                }

                // TAB 5: WERELD
                case 5: {
                    ImGui::TextColored(acc, "WERELD, WEER & TIJD CONTROLE");
                    ImGui::Separator();

                    ImGui::Text("Weer Type Kiezen:");
                    const char* weathers[] = {
                        "EXTRASUNNY", "CLEAR", "CLOUDS", "RAIN", "THUNDER", "FOGGY", "SNOW", "XMAS", "HALLOWEEN"
                    };
                    for (int w = 0; w < 9; ++w) {
                        if (ImGui::Button(weathers[w], ImVec2(110, 30))) {
                            const char* wt = weathers[w];
                            NativeQueue::Push([wt]() { Cheats::SetWeather(wt); });
                        }
                        if ((w + 1) % 3 != 0) ImGui::SameLine();
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Text("Tijd van de Dag:");

                    if (ImGui::SliderInt("Uur", &g_Config.worldHour, 0, 23)) {
                        int h = g_Config.worldHour, m = g_Config.worldMinute;
                        NativeQueue::Push([h, m]() { Cheats::SetTime(h, m); });
                    }
                    if (ImGui::SliderInt("Minuut", &g_Config.worldMinute, 0, 59)) {
                        int h = g_Config.worldHour, m = g_Config.worldMinute;
                        NativeQueue::Push([h, m]() { Cheats::SetTime(h, m); });
                    }

                    if (ImGui::Checkbox("Pauzeer Speltijd (Freeze Time)", &g_Config.worldPauseTime)) {
                        g_Config.Save(g_IniFilePath);
                    }

                    ImGui::Spacing();
                    ImGui::Text("Snelle Tijd Presets:");
                    if (ImGui::Button("Ochtend (06:00)")) {
                        g_Config.worldHour = 6; g_Config.worldMinute = 0;
                        NativeQueue::Push([]() { Cheats::SetTime(6, 0); });
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Middag (12:00)")) {
                        g_Config.worldHour = 12; g_Config.worldMinute = 0;
                        NativeQueue::Push([]() { Cheats::SetTime(12, 0); });
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Zonsondergang (19:00)")) {
                        g_Config.worldHour = 19; g_Config.worldMinute = 0;
                        NativeQueue::Push([]() { Cheats::SetTime(19, 0); });
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Nacht (00:00)")) {
                        g_Config.worldHour = 0; g_Config.worldMinute = 0;
                        NativeQueue::Push([]() { Cheats::SetTime(0, 0); });
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Text("Matrix Bullet-Time & Omgevingseffecten:");

                    if (ImGui::SliderFloat("Matrix Slow-Motion", &g_Config.worldMatrixTimeScale, 0.1f, 1.0f, "%.2fx")) {
                        float s = g_Config.worldMatrixTimeScale;
                        NativeQueue::Push([s]() { Cheats::SetTimeScale(s); });
                        g_Config.Save(g_IniFilePath);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Normale Snelheid (1.0x)")) {
                        g_Config.worldMatrixTimeScale = 1.0f;
                        NativeQueue::Push([]() { Cheats::SetTimeScale(1.0f); });
                        g_Config.Save(g_IniFilePath);
                    }

                    if (ImGui::Checkbox("Blackout Mode (Alle stads- & gebouwverlichting uit)", &g_Config.worldBlackout)) {
                        bool b = g_Config.worldBlackout;
                        NativeQueue::Push([b]() { Cheats::SetBlackout(b); });
                        g_Config.Save(g_IniFilePath);
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Text("Zwaartekracht Niveau:");
                    if (ImGui::Button("Aardse Zwaartekracht (9.8 m/s²)")) {
                        g_Config.worldGravityLevel = 0;
                        NativeQueue::Push([]() { Cheats::SetGravityLevel(0); });
                        g_Config.Save(g_IniFilePath);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Maan Zwaartekracht (2.4 m/s²)")) {
                        g_Config.worldGravityLevel = 1;
                        NativeQueue::Push([]() { Cheats::SetGravityLevel(1); });
                        g_Config.Save(g_IniFilePath);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Zeer Laag (0.1 m/s²)")) {
                        g_Config.worldGravityLevel = 2;
                        NativeQueue::Push([]() { Cheats::SetGravityLevel(2); });
                        g_Config.Save(g_IniFilePath);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Nul Zwaartekracht")) {
                        g_Config.worldGravityLevel = 3;
                        NativeQueue::Push([]() { Cheats::SetGravityLevel(3); });
                        g_Config.Save(g_IniFilePath);
                    }

                    break;
                }

                // TAB 6: INSTELLINGEN
                case 6: {
                    ImGui::TextColored(acc, "MOD MENU CONFIGURATIE & THEMA");
                    ImGui::Separator();

                    static const struct { const char* name; int vk; } hotkeys[] = {
                        { "INSERT (Standaard)", VK_INSERT },
                        { "F3", VK_F3 },
                        { "F6", VK_F6 },
                        { "F7", VK_F7 },
                        { "F8", VK_F8 },
                        { "F9", VK_F9 },
                        { "F10", VK_F10 },
                        { "F11", VK_F11 },
                        { "END", VK_END },
                        { "DELETE", VK_DELETE },
                        { "HOME", VK_HOME }
                    };

                    int currentHotkeyIdx = 0;
                    for (int k = 0; k < (int)IM_ARRAYSIZE(hotkeys); ++k) {
                        if (hotkeys[k].vk == g_Config.toggleKey) {
                            currentHotkeyIdx = k;
                            break;
                        }
                    }

                    if (ImGui::BeginCombo("Menu Toggle Toets", hotkeys[currentHotkeyIdx].name)) {
                        for (int k = 0; k < (int)IM_ARRAYSIZE(hotkeys); ++k) {
                            bool isSel = (currentHotkeyIdx == k);
                            if (ImGui::Selectable(hotkeys[k].name, isSel)) {
                                g_Config.toggleKey = hotkeys[k].vk;
                                g_Config.Save(g_IniFilePath);
                            }
                            if (isSel) ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    if (ImGui::SliderFloat("UI Schaling (DPI / Schermgrootte)", &g_Config.uiScale, 0.75f, 1.50f, "%.2fx")) {
                        g_Config.Save(g_IniFilePath);
                    }

                    if (ImGui::SliderFloat("Venster Transparantie", &g_Config.windowAlpha, 0.30f, 1.0f, "%.2f")) {
                        ApplyGlassTheme();
                        g_Config.Save(g_IniFilePath);
                    }

                    ImGui::Spacing();
                    ImGui::Text("Thema Accentkleuren:");
                    struct ColorPreset { const char* name; float r, g, b; };
                    static ColorPreset presets[] = {
                        { "Cyaan (Standaard)", 0.0f, 0.85f, 1.0f },
                        { "Neon Paars", 0.70f, 0.35f, 1.0f },
                        { "Smaragd Groen", 0.0f, 0.90f, 0.46f },
                        { "Vlammend Oranje", 1.0f, 0.55f, 0.0f },
                        { "Cyber Roze", 1.0f, 0.15f, 0.35f },
                        { "Stralend Goud", 1.0f, 0.84f, 0.0f }
                    };

                    for (int p = 0; p < (int)IM_ARRAYSIZE(presets); ++p) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(presets[p].r * 0.4f, presets[p].g * 0.4f, presets[p].b * 0.4f, 0.9f));
                        if (ImGui::Button(presets[p].name)) {
                            g_Config.accentColor[0] = presets[p].r;
                            g_Config.accentColor[1] = presets[p].g;
                            g_Config.accentColor[2] = presets[p].b;
                            ApplyGlassTheme();
                            g_Config.Save(g_IniFilePath);
                        }
                        ImGui::PopStyleColor();
                        if ((p + 1) % 3 != 0) ImGui::SameLine();
                    }

                    ImGui::Spacing();
                    if (ImGui::ColorEdit3("Aangepaste Accentkleur", g_Config.accentColor)) {
                        ApplyGlassTheme();
                        g_Config.Save(g_IniFilePath);
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    if (ImGui::Checkbox("Toon Telemetrie HUD op het scherm", &g_Config.showTelemetryHud)) {
                        g_Config.Save(g_IniFilePath);
                    }
                    if (ImGui::Checkbox("Gebruik KM/H voor Snelheidsmeter (Vink uit voor MPH)", &g_Config.speedUnitKmh)) {
                        g_Config.Save(g_IniFilePath);
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Text("Controller / Gamepad Instellingen:");
                    if (ImGui::Checkbox("Schakel Gamepad / Controller In", &g_Config.controllerEnabled)) {
                        g_Config.Save(g_IniFilePath);
                    }
                    if (ImGui::Checkbox("Rij-Sneltoetsen (Claxon Rocket Boost & Snelle Reparatie)", &g_Config.controllerDrivingShortcuts)) {
                        g_Config.Save(g_IniFilePath);
                    }
                    if (ImGui::SliderFloat("Virtuele Cursor Snelheid (Rechterstick)", &g_Config.controllerCursorSpeed, 5.0f, 35.0f, "%.1f")) {
                        g_Config.Save(g_IniFilePath);
                    }
                    ImGui::TextDisabled("Controller Knoppen: RB + D-Pad Rechts (Menu), LB/RB (Tabs), Stick (Muis), A (Klik), B (Terug)");

                    ImGui::Spacing();
                    ImGui::Separator();
                    if (ImGui::Button("Instellingen Nu Opslaan", ImVec2(180, 32))) {
                        g_Config.Save(g_IniFilePath);
                        UI::ShowToast("Instellingen opgeslagen naar config!", ImVec4(0.2f, 0.9f, 0.4f, 1.0f));
                        LOG_INFO("Saved settings to %s", g_IniFilePath.c_str());
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Herstel Standaardwaarden", ImVec2(180, 32))) {
                        g_Config.ResetToDefaults();
                        ApplyGlassTheme();
                        g_Config.Save(g_IniFilePath);
                        UI::ShowToast("Standaardinstellingen hersteld!", ImVec4(1.0f, 0.7f, 0.2f, 1.0f));
                        LOG_INFO("Reset configuration to defaults.");
                    }
                    break;
                }
            }

            ImGui::EndChild();
        }
    }
    ImGui::End();
}

void UI::Render() {
    RenderTelemetryHUD();
    RenderMainMenu();
    RenderToasts();
}
