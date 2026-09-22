#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "ScriptHookSDK.h"

struct VehicleModEntry {
    int count = 0;
    int current = -1;
};

struct VehicleModCache {
    std::mutex mtx;
    bool hasVehicle = false;
    VehicleModEntry mods[50];
};

extern VehicleModCache g_VehicleModCache;

struct TelemetryData {
    float speed = 0.0f;
    int gear = 0;
    float rpm = 0.0f;
    int health = 100;
    int maxHealth = 100;
    int armor = 0;
    int wantedLevel = 0;
    float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
    float heading = 0.0f;
    bool inVehicle = false;
    char vehicleName[64] = "None";
    float fps = 60.0f;
};

struct VehicleModelInfo {
    const char* displayName;
    const char* modelName;
    const char* category;
};

struct TeleportLocation {
    const char* name;
    const char* category;
    float x, y, z;
};

extern TelemetryData g_Telemetry;
extern const std::vector<VehicleModelInfo> g_VehicleList;
extern const std::vector<TeleportLocation> g_TeleportList;

class Cheats {
public:
    // Core Tick (executed on ScriptHookV script fiber)
    static void OnTick();

    // Player
    static void HealPlayer();
    static void AddArmor();
    static void CleanPlayerDamage();
    static void SetWantedLevel(int level);
    static void ClearWantedLevel();
    static void AddCash(int amount);
    static void SetMaxCash();
    static void RefillSpecialAbility();
    static void SuicidePlayer();
    static void RagdollPlayer();

    // Vehicle
    static void SpawnVehicle(const char* modelName, bool maxTuned = true);
    static void RepairVehicle();
    static void CleanVehicle();
    static void BoostVehicle(float force = 30.0f);
    static void InstantBrake();
    static void AutoFlipVehicle();
    static void MaxTuneVehicle();
    static void SetLicensePlate(const char* plate);

    // Vehicle Mod Shop / Los Santos Customs
    static int GetVehicleModCount(int modType);
    static int GetVehicleMod(int modType);
    static void SetVehicleMod(int modType, int modIndex);
    static void SetVehicleCustomColors(float pR, float pG, float pB, float sR, float sG, float sB);
    static void SetVehicleNeonState(bool left, bool right, bool front, bool back, float r, float g, float b);
    static void SetVehicleXenonColor(int colorIndex);
    static void SetVehicleWindowTint(int tintLevel);
    static void SetVehicleWheelType(int wheelType, int wheelIndex);
    static void SetVehicleTyreSmokeColor(float r, float g, float b);
    static void ToggleVehicleExtraMod(int modType, bool toggle);

    // Weapons
    static void GiveAllWeapons();
    static void GiveHandguns();
    static void GiveRiflesAndSMGs();
    static void GiveShotgunsAndSnipers();
    static void GiveHeavyWeapons();
    static void GiveMeleeWeapons();

    // Teleport
    static void TeleportToCoords(float x, float y, float z);
    static void TeleportToWaypoint();

    // World
    static void SetWeather(const char* weatherType);
    static void SetTime(int hour, int minute);
    static void SetGravity(int level);
    static void SetGravityLevel(int level);
    static void SetTimeScale(float scale);
    static void SetBlackout(bool enable);

    // Menu state & sound support
    static void SetInGameControlsDisabled(bool disabled);
    static void PlayFrontendSound(const char* soundName, const char* soundSet = "HUD_FRONTEND_DEFAULT_SOUNDSET");
    static void ProcessControllerInput();
};
