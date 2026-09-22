#include "Config.h"
#include <cstdio>
#include <cstdlib>
#include <string>

MenuConfig g_Config;
std::string g_IniFilePath = "EnhancedImGuiMenu.ini";

void MenuConfig::ResetToDefaults() {
    toggleKey = VK_INSERT;
    windowAlpha = 0.90f;
    accentColor[0] = 0.0f; accentColor[1] = 0.85f; accentColor[2] = 1.0f; accentColor[3] = 1.0f;
    showTelemetryHud = false;
    speedUnitKmh = true;
    uiScale = 1.0f;

    playerGodMode = false;
    playerNeverWanted = false;
    playerSuperJump = false;
    playerFastSprint = false;
    playerFastSwim = false;
    playerInvisibility = false;
    playerUnlimitedStamina = false;
    playerOneShotKill = false;
    playerNoclip = false;
    noclipSpeed = 1.0f;

    vehicleGodMode = false;
    vehicleAutoFlip = false;
    vehicleDriftMode = false;
    vehicleDriftFactor = 0.95f;
    vehicleTorqueBoost = false;
    vehicleTorqueMultiplier = 2.0f;
    vehicleRainbowPaint = false;
    vehicleSeatbelt = true;
    spawnMaxTuned = true;

    weaponsInfiniteAmmo = false;
    weaponsNoReload = false;
    weaponsExplosiveAmmo = false;
    weaponsFireBullets = false;
    weaponsRapidFire = false;
    weaponsExplosiveMelee = false;
    weaponsGravityGun = false;

    worldPauseTime = false;
    worldHour = 12;
    worldMinute = 0;
    worldGravity = 9.8f;
    worldGravityLevel = 0;
    worldMatrixTimeScale = 1.0f;
    worldBlackout = false;
    worldFreezeWeather = false;

    controllerEnabled = true;
    controllerDrivingShortcuts = true;
    controllerCursorSpeed = 15.0f;
}

void MenuConfig::Load(const std::string& iniPath) {
    g_IniFilePath = iniPath;
    char buffer[128];

    toggleKey = GetPrivateProfileIntA("Settings", "ToggleKey", VK_INSERT, iniPath.c_str());
    showTelemetryHud = GetPrivateProfileIntA("Settings", "ShowTelemetryHud", 0, iniPath.c_str()) != 0;
    speedUnitKmh = GetPrivateProfileIntA("Settings", "SpeedUnitKmh", 1, iniPath.c_str()) != 0;

    GetPrivateProfileStringA("Settings", "WindowAlpha", "0.90", buffer, sizeof(buffer), iniPath.c_str());
    windowAlpha = (float)atof(buffer);
    if (windowAlpha < 0.2f) windowAlpha = 0.2f;
    if (windowAlpha > 1.0f) windowAlpha = 1.0f;

    GetPrivateProfileStringA("Settings", "UiScale", "1.00", buffer, sizeof(buffer), iniPath.c_str());
    uiScale = (float)atof(buffer);
    if (uiScale < 0.7f) uiScale = 0.7f;
    if (uiScale > 2.0f) uiScale = 2.0f;

    GetPrivateProfileStringA("Settings", "AccentR", "0.00", buffer, sizeof(buffer), iniPath.c_str());
    accentColor[0] = (float)atof(buffer);
    GetPrivateProfileStringA("Settings", "AccentG", "0.85", buffer, sizeof(buffer), iniPath.c_str());
    accentColor[1] = (float)atof(buffer);
    GetPrivateProfileStringA("Settings", "AccentB", "1.00", buffer, sizeof(buffer), iniPath.c_str());
    accentColor[2] = (float)atof(buffer);
    accentColor[3] = 1.0f;

    controllerEnabled = GetPrivateProfileIntA("Settings", "ControllerEnabled", 1, iniPath.c_str()) != 0;
    controllerDrivingShortcuts = GetPrivateProfileIntA("Settings", "ControllerDrivingShortcuts", 1, iniPath.c_str()) != 0;
    GetPrivateProfileStringA("Settings", "ControllerCursorSpeed", "15.0", buffer, sizeof(buffer), iniPath.c_str());
    controllerCursorSpeed = (float)atof(buffer);
    if (controllerCursorSpeed < 5.0f) controllerCursorSpeed = 5.0f;
    if (controllerCursorSpeed > 40.0f) controllerCursorSpeed = 40.0f;

    // Player Cheats
    playerGodMode = GetPrivateProfileIntA("Cheats", "PlayerGodMode", 0, iniPath.c_str()) != 0;
    playerNeverWanted = GetPrivateProfileIntA("Cheats", "PlayerNeverWanted", 0, iniPath.c_str()) != 0;
    playerSuperJump = GetPrivateProfileIntA("Cheats", "PlayerSuperJump", 0, iniPath.c_str()) != 0;
    playerFastSprint = GetPrivateProfileIntA("Cheats", "PlayerFastSprint", 0, iniPath.c_str()) != 0;
    playerFastSwim = GetPrivateProfileIntA("Cheats", "PlayerFastSwim", 0, iniPath.c_str()) != 0;
    playerInvisibility = GetPrivateProfileIntA("Cheats", "PlayerInvisibility", 0, iniPath.c_str()) != 0;
    playerUnlimitedStamina = GetPrivateProfileIntA("Cheats", "PlayerUnlimitedStamina", 0, iniPath.c_str()) != 0;
    playerOneShotKill = GetPrivateProfileIntA("Cheats", "PlayerOneShotKill", 0, iniPath.c_str()) != 0;
    playerNoclip = GetPrivateProfileIntA("Cheats", "PlayerNoclip", 0, iniPath.c_str()) != 0;
    GetPrivateProfileStringA("Cheats", "NoclipSpeed", "1.0", buffer, sizeof(buffer), iniPath.c_str());
    noclipSpeed = (float)atof(buffer);
    if (noclipSpeed < 0.2f) noclipSpeed = 0.2f;
    if (noclipSpeed > 10.0f) noclipSpeed = 10.0f;

    // Vehicle Cheats
    vehicleGodMode = GetPrivateProfileIntA("Cheats", "VehicleGodMode", 0, iniPath.c_str()) != 0;
    vehicleAutoFlip = GetPrivateProfileIntA("Cheats", "VehicleAutoFlip", 0, iniPath.c_str()) != 0;
    vehicleDriftMode = GetPrivateProfileIntA("Cheats", "VehicleDriftMode", 0, iniPath.c_str()) != 0;
    GetPrivateProfileStringA("Cheats", "VehicleDriftFactor", "0.95", buffer, sizeof(buffer), iniPath.c_str());
    vehicleDriftFactor = (float)atof(buffer);
    vehicleTorqueBoost = GetPrivateProfileIntA("Cheats", "VehicleTorqueBoost", 0, iniPath.c_str()) != 0;
    GetPrivateProfileStringA("Cheats", "VehicleTorqueMultiplier", "2.0", buffer, sizeof(buffer), iniPath.c_str());
    vehicleTorqueMultiplier = (float)atof(buffer);
    if (vehicleTorqueMultiplier < 1.0f) vehicleTorqueMultiplier = 1.0f;
    if (vehicleTorqueMultiplier > 20.0f) vehicleTorqueMultiplier = 20.0f;
    vehicleRainbowPaint = GetPrivateProfileIntA("Cheats", "VehicleRainbowPaint", 0, iniPath.c_str()) != 0;
    vehicleSeatbelt = GetPrivateProfileIntA("Cheats", "VehicleSeatbelt", 1, iniPath.c_str()) != 0;
    spawnMaxTuned = GetPrivateProfileIntA("Cheats", "SpawnMaxTuned", 1, iniPath.c_str()) != 0;

    // Weapon Cheats
    weaponsInfiniteAmmo = GetPrivateProfileIntA("Cheats", "WeaponsInfiniteAmmo", 0, iniPath.c_str()) != 0;
    weaponsNoReload = GetPrivateProfileIntA("Cheats", "WeaponsNoReload", 0, iniPath.c_str()) != 0;
    weaponsExplosiveAmmo = GetPrivateProfileIntA("Cheats", "WeaponsExplosiveAmmo", 0, iniPath.c_str()) != 0;
    weaponsFireBullets = GetPrivateProfileIntA("Cheats", "WeaponsFireBullets", 0, iniPath.c_str()) != 0;
    weaponsRapidFire = GetPrivateProfileIntA("Cheats", "WeaponsRapidFire", 0, iniPath.c_str()) != 0;
    weaponsExplosiveMelee = GetPrivateProfileIntA("Cheats", "WeaponsExplosiveMelee", 0, iniPath.c_str()) != 0;
    weaponsGravityGun = GetPrivateProfileIntA("Cheats", "WeaponsGravityGun", 0, iniPath.c_str()) != 0;

    // World Cheats
    worldPauseTime = GetPrivateProfileIntA("Cheats", "WorldPauseTime", 0, iniPath.c_str()) != 0;
    worldHour = GetPrivateProfileIntA("Cheats", "WorldHour", 12, iniPath.c_str());
    worldMinute = GetPrivateProfileIntA("Cheats", "WorldMinute", 0, iniPath.c_str());
    worldGravityLevel = GetPrivateProfileIntA("Cheats", "WorldGravityLevel", 0, iniPath.c_str());
    if (worldGravityLevel < 0 || worldGravityLevel > 3) worldGravityLevel = 0;

    GetPrivateProfileStringA("Cheats", "WorldMatrixTimeScale", "1.00", buffer, sizeof(buffer), iniPath.c_str());
    worldMatrixTimeScale = (float)atof(buffer);
    if (worldMatrixTimeScale < 0.05f) worldMatrixTimeScale = 0.05f;
    if (worldMatrixTimeScale > 1.0f) worldMatrixTimeScale = 1.0f;

    worldBlackout = GetPrivateProfileIntA("Cheats", "WorldBlackout", 0, iniPath.c_str()) != 0;
    worldFreezeWeather = GetPrivateProfileIntA("Cheats", "WorldFreezeWeather", 0, iniPath.c_str()) != 0;
}

void MenuConfig::Save(const std::string& iniPath) const {
    char buffer[64];

    snprintf(buffer, sizeof(buffer), "%d", toggleKey);
    WritePrivateProfileStringA("Settings", "ToggleKey", buffer, iniPath.c_str());

    snprintf(buffer, sizeof(buffer), "%d", showTelemetryHud ? 1 : 0);
    WritePrivateProfileStringA("Settings", "ShowTelemetryHud", buffer, iniPath.c_str());

    snprintf(buffer, sizeof(buffer), "%d", speedUnitKmh ? 1 : 0);
    WritePrivateProfileStringA("Settings", "SpeedUnitKmh", buffer, iniPath.c_str());

    snprintf(buffer, sizeof(buffer), "%.2f", windowAlpha);
    WritePrivateProfileStringA("Settings", "WindowAlpha", buffer, iniPath.c_str());

    snprintf(buffer, sizeof(buffer), "%.2f", uiScale);
    WritePrivateProfileStringA("Settings", "UiScale", buffer, iniPath.c_str());

    snprintf(buffer, sizeof(buffer), "%.2f", accentColor[0]);
    WritePrivateProfileStringA("Settings", "AccentR", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%.2f", accentColor[1]);
    WritePrivateProfileStringA("Settings", "AccentG", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%.2f", accentColor[2]);
    WritePrivateProfileStringA("Settings", "AccentB", buffer, iniPath.c_str());

    snprintf(buffer, sizeof(buffer), "%d", controllerEnabled ? 1 : 0);
    WritePrivateProfileStringA("Settings", "ControllerEnabled", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", controllerDrivingShortcuts ? 1 : 0);
    WritePrivateProfileStringA("Settings", "ControllerDrivingShortcuts", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%.1f", controllerCursorSpeed);
    WritePrivateProfileStringA("Settings", "ControllerCursorSpeed", buffer, iniPath.c_str());

    // Player
    snprintf(buffer, sizeof(buffer), "%d", playerGodMode ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "PlayerGodMode", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", playerNeverWanted ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "PlayerNeverWanted", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", playerSuperJump ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "PlayerSuperJump", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", playerFastSprint ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "PlayerFastSprint", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", playerFastSwim ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "PlayerFastSwim", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", playerInvisibility ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "PlayerInvisibility", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", playerUnlimitedStamina ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "PlayerUnlimitedStamina", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", playerOneShotKill ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "PlayerOneShotKill", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", playerNoclip ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "PlayerNoclip", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%.2f", noclipSpeed);
    WritePrivateProfileStringA("Cheats", "NoclipSpeed", buffer, iniPath.c_str());

    // Vehicle
    snprintf(buffer, sizeof(buffer), "%d", vehicleGodMode ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "VehicleGodMode", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", vehicleAutoFlip ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "VehicleAutoFlip", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", vehicleDriftMode ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "VehicleDriftMode", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%.2f", vehicleDriftFactor);
    WritePrivateProfileStringA("Cheats", "VehicleDriftFactor", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", vehicleTorqueBoost ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "VehicleTorqueBoost", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%.2f", vehicleTorqueMultiplier);
    WritePrivateProfileStringA("Cheats", "VehicleTorqueMultiplier", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", vehicleRainbowPaint ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "VehicleRainbowPaint", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", vehicleSeatbelt ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "VehicleSeatbelt", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", spawnMaxTuned ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "SpawnMaxTuned", buffer, iniPath.c_str());

    // Weapons
    snprintf(buffer, sizeof(buffer), "%d", weaponsInfiniteAmmo ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "WeaponsInfiniteAmmo", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", weaponsNoReload ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "WeaponsNoReload", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", weaponsExplosiveAmmo ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "WeaponsExplosiveAmmo", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", weaponsFireBullets ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "WeaponsFireBullets", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", weaponsRapidFire ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "WeaponsRapidFire", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", weaponsExplosiveMelee ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "WeaponsExplosiveMelee", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", weaponsGravityGun ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "WeaponsGravityGun", buffer, iniPath.c_str());

    // World
    snprintf(buffer, sizeof(buffer), "%d", worldPauseTime ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "WorldPauseTime", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", worldHour);
    WritePrivateProfileStringA("Cheats", "WorldHour", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", worldMinute);
    WritePrivateProfileStringA("Cheats", "WorldMinute", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", worldGravityLevel);
    WritePrivateProfileStringA("Cheats", "WorldGravityLevel", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%.2f", worldMatrixTimeScale);
    WritePrivateProfileStringA("Cheats", "WorldMatrixTimeScale", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", worldBlackout ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "WorldBlackout", buffer, iniPath.c_str());
    snprintf(buffer, sizeof(buffer), "%d", worldFreezeWeather ? 1 : 0);
    WritePrivateProfileStringA("Cheats", "WorldFreezeWeather", buffer, iniPath.c_str());
}
