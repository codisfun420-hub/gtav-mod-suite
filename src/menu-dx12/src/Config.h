#pragma once
#include <windows.h>
#include <string>

struct MenuConfig {
    int toggleKey = VK_INSERT;
    float windowAlpha = 0.90f;
    float accentColor[4] = { 0.0f, 0.85f, 1.0f, 1.0f }; // Modern Vibrant Cyan
    bool showTelemetryHud = false;
    bool speedUnitKmh = true;
    float uiScale = 1.0f;

    // Player Cheats
    bool playerGodMode = false;
    bool playerNeverWanted = false;
    bool playerSuperJump = false;
    bool playerFastSprint = false;
    bool playerFastSwim = false;
    bool playerInvisibility = false;
    bool playerUnlimitedStamina = false;
    bool playerOneShotKill = false;
    bool playerNoclip = false;
    float noclipSpeed = 1.0f;

    // Vehicle Cheats
    bool vehicleGodMode = false;
    bool vehicleAutoFlip = false;
    bool vehicleDriftMode = false;
    float vehicleDriftFactor = 0.95f;
    bool vehicleTorqueBoost = false;
    float vehicleTorqueMultiplier = 2.0f;
    bool vehicleRainbowPaint = false;
    bool vehicleSeatbelt = true;
    bool spawnMaxTuned = true;

    // Weapon Cheats
    bool weaponsInfiniteAmmo = false;
    bool weaponsNoReload = false;
    bool weaponsExplosiveAmmo = false;
    bool weaponsFireBullets = false;
    bool weaponsRapidFire = false;
    bool weaponsExplosiveMelee = false;
    bool weaponsGravityGun = false;

    // World Cheats
    bool worldPauseTime = false;
    int worldHour = 12;
    int worldMinute = 0;
    float worldGravity = 9.8f;
    int worldGravityLevel = 0; // 0=Earth (9.8), 1=Moon (2.4), 2=Very Low (0.1), 3=Zero (0.0)
    float worldMatrixTimeScale = 1.0f; // 0.1 to 1.0
    bool worldBlackout = false;
    bool worldFreezeWeather = false;
    // Controller / Gamepad
    bool controllerEnabled = true;
    bool controllerDrivingShortcuts = true;
    float controllerCursorSpeed = 15.0f;

    void Load(const std::string& iniPath);
    void Save(const std::string& iniPath) const;
    void ResetToDefaults();
};

extern MenuConfig g_Config;
extern std::string g_IniFilePath;
