#include "Cheats.h"
#include "Config.h"
#include "Logger.h"
#include "UI.h"
#include <cmath>
#include <cstdio>
#include <algorithm>

TelemetryData g_Telemetry;
VehicleModCache g_VehicleModCache;

const std::vector<VehicleModelInfo> g_VehicleList = {
    // Supers
    { "Adder", "adder", "Supers" },
    { "Progen T20", "t20", "Supers" },
    { "Pegassi Zentorno", "zentorno", "Supers" },
    { "Pegassi Osiris", "osiris", "Supers" },
    { "Overflod Entity XF", "entityxf", "Supers" },
    { "Pegassi Reaper", "reaper", "Supers" },
    { "Pegassi Tempesta", "tempesta", "Supers" },
    { "Grotti X80 Proto", "proto", "Supers" },
    { "Vigilante (Batmobile)", "vigilante", "Supers" },
    { "Truffade Thrax", "thrax", "Supers" },
    { "Benefactor Krieger", "krieger", "Supers" },
    { "Principe Deveste Eight", "deveste", "Supers" },
    { "Pegassi Ignus", "ignus", "Supers" },
    { "Grotti Turismo R", "turismor", "Supers" },

    // Sports
    { "Bravado Banshee", "banshee", "Sports" },
    { "Karin Sultan", "sultan", "Sports" },
    { "Annis Elegy RH8", "elegy2", "Sports" },
    { "Dinka Jester", "jester", "Sports" },
    { "Dewbauchee Massacro", "massacro", "Sports" },
    { "Grotti Carbonizzare", "carbonizzare", "Sports" },
    { "Invetero Coquette", "coquette", "Sports" },
    { "Benefactor Feltzer", "feltzer", "Sports" },
    { "Pfister Comet", "comet2", "Sports" },
    { "Karin Kuruma (Armored)", "kuruma2", "Sports" },
    { "Obey 9F", "ninef", "Sports" },

    // Muscle
    { "Vapid Dominator", "dominator", "Muscle" },
    { "Declasse Sabre Turbo", "sabregt", "Muscle" },
    { "Imponte Phoenix", "phoenix", "Muscle" },
    { "Imponte Ruiner", "ruiner", "Muscle" },
    { "Bravado Gauntlet", "gauntlet", "Muscle" },

    // Off-Road
    { "BF Injection", "bfinjection", "Off-Road" },
    { "Canis Kamacho", "kamacho", "Off-Road" },
    { "BF Bifta", "bifta", "Off-Road" },
    { "Benefactor Dubsta 6x6", "dubsta3", "Off-Road" },
    { "Vapid Sandking XL", "sandking", "Off-Road" },
    { "Coil Brawler", "brawler", "Off-Road" },
    { "Karin Rebel", "rebel2", "Off-Road" },
    { "Vapid Trophy Truck", "trophytruck", "Off-Road" },

    // Helicopters
    { "Nagasaki Buzzard", "buzzard", "Helicopters" },
    { "Buzzard Attack Chopper", "buzzard2", "Helicopters" },
    { "Savage Gunship", "savage", "Helicopters" },
    { "FH-1 Hunter", "hunter", "Helicopters" },
    { "Akula Stealth Chopper", "akula", "Helicopters" },
    { "Buckingham Maverick", "maverick", "Helicopters" },
    { "Western Cargobob", "cargobob", "Helicopters" },

    // Planes
    { "Mammoth Hydra (VTOL Jet)", "hydra", "Planes" },
    { "P-996 LAZER Jet", "lazer", "Planes" },
    { "Titan Transport", "titan", "Planes" },
    { "Western Besra Jet", "besra", "Planes" },
    { "Buckingham Luxor Deluxe", "luxor2", "Planes" },
    { "Mallard Stunt Plane", "mallard", "Planes" },

    // Motorcycles
    { "Pegassi Bati 801", "bati", "Motorcycles" },
    { "Dinka Akuma", "akuma", "Motorcycles" },
    { "Dinka Double-T", "double", "Motorcycles" },
    { "Shitzu Hakuchou", "hakuchou", "Motorcycles" },
    { "Shitzu Hakuchou Drag", "hakuchou2", "Motorcycles" },
    { "Maibatsu Sanchez", "sanchez", "Motorcycles" },
    { "Pegassi Vortex", "vortex", "Motorcycles" },
    { "Western Bagger", "bagger", "Motorcycles" },

    // Service & Emergency
    { "Police Cruiser (Vapid)", "police", "Emergency" },
    { "Police Buffalo", "police2", "Emergency" },
    { "Police Interceptor", "police3", "Emergency" },
    { "Sheriff Cruiser", "sheriff", "Emergency" },
    { "FIB Buffalo", "fbi", "Emergency" },
    { "FIB Granger", "fbi2", "Emergency" },
    { "Ambulance", "ambulance", "Emergency" },
    { "Fire Truck", "firetruk", "Emergency" },
    { "Downtown Cab (Taxi)", "taxi", "Emergency" }
};

const std::vector<TeleportLocation> g_TeleportList = {
    { "Mount Chiliad Summit", "Scenic Peaks", 452.1f, 5642.5f, 780.0f },
    { "Mount Gordo Peak", "Scenic Peaks", 2894.0f, 4920.0f, 148.0f },
    { "Maze Bank Tower Roof", "Skyline Landmarks", -75.0f, -818.0f, 326.0f },
    { "FIB Building Rooftop", "Skyline Landmarks", 136.0f, -750.0f, 262.0f },
    { "Galileo Observatory", "Skyline Landmarks", -438.0f, 1073.0f, 352.0f },
    { "Vinewood Sign", "Skyline Landmarks", 711.0f, 1198.0f, 348.0f },
    { "Los Santos Int. Airport (LSIA)", "Airfields", -1037.0f, -2737.0f, 13.8f },
    { "Sandy Shores Airfield", "Airfields", 1734.0f, 3711.0f, 34.0f },
    { "Fort Zancudo ATC Tower", "Military", -2360.0f, 3244.0f, 92.9f },
    { "Paleto Bay Pier", "Coastal", -275.0f, 6636.0f, 7.5f },
    { "Del Perro Pier", "Coastal", -1685.0f, -1072.0f, 13.1f },
    { "Michael's Rockford Hills Mansion", "Safehouses", -813.0f, 178.0f, 72.0f },
    { "Franklin's Vinewood Hills Villa", "Safehouses", 7.0f, 528.0f, 170.0f },
    { "Trevor's Sandy Shores Trailer", "Safehouses", 1973.0f, 3816.0f, 33.4f },
    { "Legion Square", "Urban", 150.0f, -1040.0f, 29.0f },
    { "The Diamond Casino & Resort", "Luxury", 925.0f, 46.0f, 81.1f },
    { "Humane Labs & Research Facility", "Interiors & Facilities", 3560.0f, 3674.0f, 28.1f }
};

static bool s_PrevGodMode = false;
static bool s_PrevFastSprint = false;
static bool s_PrevFastSwim = false;
static bool s_PrevInvisibility = false;
static bool s_PrevVehicleGod = false;
static bool s_PrevInfAmmo = false;
static bool s_PrevNoReload = false;
static bool s_PrevRapidFire = false;
static bool s_PrevOneShotKill = false;
static bool s_PrevPauseTime = false;
static bool s_PrevBlackout = false;
static bool s_PrevNoclip = false;
static float s_PrevTimeScale = 1.0f;
static float s_RainbowHue = 0.0f;
static Vehicle s_PrevVeh = 0;
static DWORD s_LastModCacheTick = 0;

static void HSVtoRGB(float h, float s, float v, int& r, int& g, int& b) {
    float c = v * s;
    float x = c * (1.0f - fabsf(fmodf(h * 6.0f, 2.0f) - 1.0f));
    float m = v - c;
    float r1 = 0, g1 = 0, b1 = 0;
    if (h < 1.0f / 6.0f) { r1 = c; g1 = x; }
    else if (h < 2.0f / 6.0f) { r1 = x; g1 = c; }
    else if (h < 3.0f / 6.0f) { g1 = c; b1 = x; }
    else if (h < 4.0f / 6.0f) { g1 = x; b1 = c; }
    else if (h < 5.0f / 6.0f) { r1 = x; b1 = c; }
    else { r1 = c; b1 = x; }
    r = (int)((r1 + m) * 255.0f);
    g = (int)((g1 + m) * 255.0f);
    b = (int)((b1 + m) * 255.0f);
}

void Cheats::PlayFrontendSound(const char* soundName, const char* soundSet) {
    invoke<void>(0x67C540AA08E4A6F5, -1, soundName, soundSet, FALSE); // PLAY_SOUND_FRONTEND
}

void Cheats::OnTick() {
    Player player = invoke<Player>(0x4F8644AF03D0E0D6); // PLAYER_ID
    Ped ped = invoke<Ped>(0xD80958FC74E988A6); // PLAYER_PED_ID

    if (!invoke<BOOL>(0x7239B21A38F536BA, ped)) return; // DOES_ENTITY_EXIST

    // 1. Continuous Player Cheats
    if (g_Config.playerGodMode) {
        invoke<void>(0x239528EACDC3E7DE, player, TRUE); // SET_PLAYER_INVINCIBLE
        invoke<void>(0x3882114BDE571AD4, ped, TRUE, FALSE); // SET_ENTITY_INVINCIBLE
        invoke<void>(0x6B76DC1F3AE6E6A3, ped, 200, 0, 0); // SET_ENTITY_HEALTH
        s_PrevGodMode = true;
    } else if (s_PrevGodMode) {
        invoke<void>(0x239528EACDC3E7DE, player, FALSE);
        invoke<void>(0x3882114BDE571AD4, ped, FALSE, FALSE);
        s_PrevGodMode = false;
    }

    if (g_Config.playerNeverWanted) {
        invoke<void>(0xB302540597885499, player); // CLEAR_PLAYER_WANTED_LEVEL
        invoke<void>(0x39FF19C64EF7DA5B, player, 0, FALSE); // SET_PLAYER_WANTED_LEVEL
    }

    if (g_Config.playerSuperJump) {
        invoke<void>(0x57FFF03E423A4C0B, player); // SET_SUPER_JUMP_THIS_FRAME
    }

    if (g_Config.playerFastSprint) {
        invoke<void>(0x6DB47AA77FD94E09, player, 1.49f); // SET_RUN_SPRINT_MULTIPLIER_FOR_PLAYER
        s_PrevFastSprint = true;
    } else if (s_PrevFastSprint) {
        invoke<void>(0x6DB47AA77FD94E09, player, 1.0f);
        s_PrevFastSprint = false;
    }

    if (g_Config.playerFastSwim) {
        invoke<void>(0xA91C6F0FF7D16A13, player, 1.49f); // SET_SWIM_MULTIPLIER_FOR_PLAYER
        s_PrevFastSwim = true;
    } else if (s_PrevFastSwim) {
        invoke<void>(0xA91C6F0FF7D16A13, player, 1.0f);
        s_PrevFastSwim = false;
    }

    if (g_Config.playerUnlimitedStamina) {
        invoke<void>(0xA6F312FCCE9C1DFE, player); // RESET_PLAYER_STAMINA
    }

    if (g_Config.playerInvisibility) {
        invoke<void>(0xEA1C610A04DB6BBB, ped, FALSE, 0); // SET_ENTITY_VISIBLE
        s_PrevInvisibility = true;
    } else if (s_PrevInvisibility) {
        invoke<void>(0xEA1C610A04DB6BBB, ped, TRUE, 0);
        s_PrevInvisibility = false;
    }

    if (g_Config.playerOneShotKill) {
        invoke<void>(0xCE07B9F7817AADA3, player, 1000.0f); // SET_PLAYER_WEAPON_DAMAGE_MODIFIER
        s_PrevOneShotKill = true;
    } else if (s_PrevOneShotKill) {
        invoke<void>(0xCE07B9F7817AADA3, player, 1.0f);
        s_PrevOneShotKill = false;
    }

    bool inVeh = invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE); // IS_PED_IN_ANY_VEHICLE
    Vehicle veh = inVeh ? invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE) : 0; // GET_VEHICLE_PED_IS_IN

    // Noclip / Freecam
    if (g_Config.playerNoclip) {
        Entity targetEnt = inVeh ? veh : ped;
        if (targetEnt && invoke<BOOL>(0x7239B21A38F536BA, targetEnt)) {
            invoke<void>(0x1A9205C1B9EE827F, targetEnt, FALSE, FALSE); // SET_ENTITY_COLLISION
            invoke<void>(0x1C99BB7B6E96D16F, targetEnt, 0.0f, 0.0f, 0.0f); // SET_ENTITY_VELOCITY

            Vector3 camRot = invoke<Vector3>(0x837765A25378F0BB, 2); // GET_GAMEPLAY_CAM_ROT
            Vector3 curPos = invoke<Vector3>(0x3FEF770D40960D5A, targetEnt, TRUE); // GET_ENTITY_COORDS

            float pitch = camRot.x * 0.01745329251f;
            float yaw = camRot.z * 0.01745329251f;

            float cosPitch = cosf(pitch);
            float sinPitch = sinf(pitch);
            float cosYaw = cosf(yaw);
            float sinYaw = sinf(yaw);

            float fwdX = -sinYaw * cosPitch;
            float fwdY = cosYaw * cosPitch;
            float fwdZ = sinPitch;

            float rightX = cosYaw;
            float rightY = sinYaw;

            float moveSpeed = g_Config.noclipSpeed * 0.8f;
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) moveSpeed *= 2.5f;

            if (GetAsyncKeyState('W') & 0x8000) {
                curPos.x += fwdX * moveSpeed;
                curPos.y += fwdY * moveSpeed;
                curPos.z += fwdZ * moveSpeed;
            }
            if (GetAsyncKeyState('S') & 0x8000) {
                curPos.x -= fwdX * moveSpeed;
                curPos.y -= fwdY * moveSpeed;
                curPos.z -= fwdZ * moveSpeed;
            }
            if (GetAsyncKeyState('D') & 0x8000) {
                curPos.x += rightX * moveSpeed;
                curPos.y += rightY * moveSpeed;
            }
            if (GetAsyncKeyState('A') & 0x8000) {
                curPos.x -= rightX * moveSpeed;
                curPos.y -= rightY * moveSpeed;
            }
            if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
                curPos.z += moveSpeed * 0.7f;
            }
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
                curPos.z -= moveSpeed * 0.7f;
            }

            invoke<void>(0x239A3351AC1DA385, targetEnt, curPos.x, curPos.y, curPos.z, TRUE, TRUE, TRUE); // SET_ENTITY_COORDS_NO_OFFSET
            invoke<void>(0x8E2530AA8ADA980E, targetEnt, camRot.z); // SET_ENTITY_HEADING
            s_PrevNoclip = true;
        }
    } else if (s_PrevNoclip) {
        Entity targetEnt = inVeh ? veh : ped;
        if (targetEnt && invoke<BOOL>(0x7239B21A38F536BA, targetEnt)) {
            invoke<void>(0x1A9205C1B9EE827F, targetEnt, TRUE, TRUE); // SET_ENTITY_COLLISION
        }
        s_PrevNoclip = false;
    }

    // 2. Continuous Vehicle Cheats
    if (inVeh && veh) {
        if (g_Config.vehicleGodMode) {
            invoke<void>(0x3882114BDE571AD4, veh, TRUE, FALSE); // SET_ENTITY_INVINCIBLE
            invoke<void>(0x115722B1B9C14C1C, veh); // SET_VEHICLE_FIXED
            invoke<void>(0x79D3B596FE44EE8B, veh, 0.0f); // SET_VEHICLE_DIRT_LEVEL
            s_PrevVehicleGod = true;
        } else if (s_PrevVehicleGod) {
            invoke<void>(0x3882114BDE571AD4, veh, FALSE, FALSE);
            s_PrevVehicleGod = false;
        }

        // Vehicle Seatbelt (prevent flying out of windshield)
        if (g_Config.vehicleSeatbelt) {
            invoke<void>(0x1913FE4CBF41C463, ped, 32, FALSE); // SET_PED_CONFIG_FLAG (32 = Can fly through windscreen)
        }

        // Vehicle Drift Mode
        if (g_Config.vehicleDriftMode) {
            invoke<void>(0x222FF6A823D122E2, veh, TRUE); // SET_VEHICLE_REDUCE_GRIP
        }

        // Vehicle Torque / Power Multiplier
        if (g_Config.vehicleTorqueBoost) {
            invoke<void>(0xB59E4BD37AE292DB, veh, g_Config.vehicleTorqueMultiplier); // SET_VEHICLE_CHEAT_POWER_INCREASE
        }

        // Vehicle Auto-Flip Upright if inverted
        if (g_Config.vehicleAutoFlip) {
            float roll = invoke<float>(0x831E0242595560DF, veh); // GET_ENTITY_ROLL
            if (fabsf(roll) > 75.0f) {
                invoke<void>(0x49733E92263139D1, veh); // SET_VEHICLE_ON_GROUND_PROPERLY
            }
        }

        // Vehicle Rainbow RGB Paint
        if (g_Config.vehicleRainbowPaint) {
            s_RainbowHue += 0.005f;
            if (s_RainbowHue > 1.0f) s_RainbowHue -= 1.0f;
            int r = 0, g = 0, b = 0;
            HSVtoRGB(s_RainbowHue, 1.0f, 1.0f, r, g, b);
            invoke<void>(0x7141766F91D15BEA, veh, r, g, b); // SET_VEHICLE_CUSTOM_PRIMARY_COLOUR
            invoke<void>(0x36CED73BFED89754, veh, r, g, b); // SET_VEHICLE_CUSTOM_SECONDARY_COLOUR
        }
    } else {
        s_PrevVehicleGod = false;
    }

    // 3. Continuous Weapon Cheats
    if (g_Config.weaponsInfiniteAmmo) {
        invoke<void>(0x3EDCB0505123623B, ped, TRUE, 0); // SET_PED_INFINITE_AMMO
        s_PrevInfAmmo = true;
    } else if (s_PrevInfAmmo) {
        invoke<void>(0x3EDCB0505123623B, ped, FALSE, 0);
        s_PrevInfAmmo = false;
    }

    if (g_Config.weaponsNoReload) {
        invoke<void>(0x183DADC6AA953186, ped, TRUE); // SET_PED_INFINITE_AMMO_CLIP
        s_PrevNoReload = true;
    } else if (s_PrevNoReload) {
        invoke<void>(0x183DADC6AA953186, ped, FALSE);
        s_PrevNoReload = false;
    }

    if (g_Config.weaponsExplosiveAmmo) {
        invoke<void>(0xA66C71C98D5F2CFB, player); // SET_EXPLOSIVE_AMMO_THIS_FRAME
    }
    if (g_Config.weaponsFireBullets) {
        invoke<void>(0x11879CDD803D30F4, player); // SET_FIRE_AMMO_THIS_FRAME
    }
    if (g_Config.weaponsExplosiveMelee) {
        invoke<void>(0xFF1BED81BFDC0FE0, player); // SET_EXPLOSIVE_MELEE_THIS_FRAME
    }
    if (g_Config.weaponsRapidFire) {
        invoke<void>(0x614DA022990752DC, ped, 1000); // SET_PED_SHOOT_RATE
        s_PrevRapidFire = true;
    } else if (s_PrevRapidFire) {
        invoke<void>(0x614DA022990752DC, ped, 100);
        s_PrevRapidFire = false;
    }

    // Gravity Gun Physics Manipulator
    static Entity s_GrabbedEntity = 0;
    if (g_Config.weaponsGravityGun) {
        if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
            if (!s_GrabbedEntity) {
                Entity aimed = 0;
                if (invoke<BOOL>(0x2975C866E6713290, player, &aimed)) { // GET_ENTITY_PLAYER_IS_FREE_AIMING_AT
                    if (aimed && invoke<BOOL>(0x7239B21A38F536BA, aimed)) { // DOES_ENTITY_EXIST
                        s_GrabbedEntity = aimed;
                    }
                }
            }
            if (s_GrabbedEntity && invoke<BOOL>(0x7239B21A38F536BA, s_GrabbedEntity)) {
                Vector3 camRot = invoke<Vector3>(0x837765A25378F0BB, 2); // GET_GAMEPLAY_CAM_ROT
                Vector3 pPos = invoke<Vector3>(0x3FEF770D40960D5A, ped, TRUE); // GET_ENTITY_COORDS
                float pitch = camRot.x * 0.01745329251f;
                float yaw = camRot.z * 0.01745329251f;
                float dist = 7.0f;
                float targetX = pPos.x - sinf(yaw) * cosf(pitch) * dist;
                float targetY = pPos.y + cosf(yaw) * cosf(pitch) * dist;
                float targetZ = pPos.z + sinf(pitch) * dist + 1.0f;
                invoke<void>(0x1C99BB7B6E96D16F, s_GrabbedEntity, 0.0f, 0.0f, 0.0f); // SET_ENTITY_VELOCITY
                invoke<void>(0x239A3351AC1DA385, s_GrabbedEntity, targetX, targetY, targetZ, TRUE, TRUE, TRUE); // SET_ENTITY_COORDS_NO_OFFSET

                // Fire launch on Left Click
                if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                    float force = 90.0f;
                    float fx = -sinf(yaw) * cosf(pitch) * force;
                    float fy = cosf(yaw) * cosf(pitch) * force;
                    float fz = sinf(pitch) * force;
                    invoke<void>(0x1C99BB7B6E96D16F, s_GrabbedEntity, fx, fy, fz); // SET_ENTITY_VELOCITY
                    s_GrabbedEntity = 0;
                }
            }
        } else {
            s_GrabbedEntity = 0;
        }
    }

    // 4. World & Environment Cheats
    if (g_Config.worldPauseTime) {
        invoke<void>(0x4055E40BD2DBEC1D, TRUE); // PAUSE_CLOCK(TRUE)
        s_PrevPauseTime = true;
    } else if (s_PrevPauseTime) {
        invoke<void>(0x4055E40BD2DBEC1D, FALSE);
        s_PrevPauseTime = false;
    }

    if (g_Config.worldGravityLevel >= 0 && g_Config.worldGravityLevel <= 3) {
        invoke<void>(0x740E14FAD5842351, g_Config.worldGravityLevel); // SET_GRAVITY_LEVEL
    }

    if (fabsf(g_Config.worldMatrixTimeScale - s_PrevTimeScale) > 0.01f) {
        invoke<void>(0x1D408577D440E81E, g_Config.worldMatrixTimeScale); // SET_TIME_SCALE
        s_PrevTimeScale = g_Config.worldMatrixTimeScale;
    }

    if (g_Config.worldBlackout != s_PrevBlackout) {
        invoke<void>(0x1268615ACE24D504, g_Config.worldBlackout ? TRUE : FALSE); // SET_ARTIFICIAL_LIGHTS_STATE
        s_PrevBlackout = g_Config.worldBlackout;
    }

    // 5. Update Telemetry
    Vector3 coords = invoke<Vector3>(0x3FEF770D40960D5A, ped, TRUE); // GET_ENTITY_COORDS
    g_Telemetry.posX = coords.x;
    g_Telemetry.posY = coords.y;
    g_Telemetry.posZ = coords.z;
    g_Telemetry.heading = invoke<float>(0xE83D4F9BA2A38914, ped); // GET_ENTITY_HEADING
    g_Telemetry.health = invoke<int>(0xEEF059FAD016D209, ped); // GET_ENTITY_HEALTH
    g_Telemetry.maxHealth = invoke<int>(0x15D757606D170C3C, ped); // GET_ENTITY_MAX_HEALTH
    g_Telemetry.armor = invoke<int>(0x9483AF821605B1D8, ped); // GET_PED_ARMOUR
    g_Telemetry.wantedLevel = invoke<int>(0xE28E54788CE8F12D, player); // GET_PLAYER_WANTED_LEVEL
    g_Telemetry.inVehicle = inVeh;

    if (inVeh && veh) {
        float speedMPS = invoke<float>(0xD5037BA82E12416F, veh); // GET_ENTITY_SPEED
        g_Telemetry.speed = g_Config.speedUnitKmh ? (speedMPS * 3.6f) : (speedMPS * 2.236936f);

        // Cache vehicle mods thread-safely for UI thread consumption
        DWORD now = GetTickCount();
        if (veh != s_PrevVeh || (UI::s_MenuOpen && (now - s_LastModCacheTick > 250))) {
            s_PrevVeh = veh;
            s_LastModCacheTick = now;
            invoke<void>(0x1F2AA07F00B3217A, veh, 0); // SET_VEHICLE_MOD_KIT
            VehicleModEntry localMods[50];
            for (int m = 0; m < 50; ++m) {
                localMods[m].count = invoke<int>(0xE38E9162A2500646, veh, m); // GET_NUM_VEHICLE_MODS
                localMods[m].current = invoke<int>(0x772960298DA26FDB, veh, m); // GET_VEHICLE_MOD
            }
            {
                std::lock_guard<std::mutex> lock(g_VehicleModCache.mtx);
                g_VehicleModCache.hasVehicle = true;
                for (int m = 0; m < 50; ++m) {
                    g_VehicleModCache.mods[m] = localMods[m];
                }
            }
        }
    } else {
        float speedMPS = invoke<float>(0xD5037BA82E12416F, ped);
        g_Telemetry.speed = g_Config.speedUnitKmh ? (speedMPS * 3.6f) : (speedMPS * 2.236936f);

        s_PrevVeh = 0;
        {
            std::lock_guard<std::mutex> lock(g_VehicleModCache.mtx);
            if (g_VehicleModCache.hasVehicle) {
                g_VehicleModCache.hasVehicle = false;
                for (int m = 0; m < 50; ++m) {
                    g_VehicleModCache.mods[m].count = 0;
                    g_VehicleModCache.mods[m].current = -1;
                }
            }
        }
    }


    // 6. Process Gamepad / Controller input & shortcuts
    ProcessControllerInput();
}

void Cheats::HealPlayer() {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (invoke<BOOL>(0x7239B21A38F536BA, ped)) {
        invoke<void>(0x6B76DC1F3AE6E6A3, ped, 200, 0, 0); // SET_ENTITY_HEALTH
        CleanPlayerDamage();
        PlayFrontendSound("CONFIRM_BEEP");
        UI::ShowToast("Speler: Volledig genezen (100% HP)", ImVec4(0.2f, 0.9f, 0.4f, 1.0f));
        LOG_INFO("Player healed to maximum health.");
    }
}

void Cheats::AddArmor() {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (invoke<BOOL>(0x7239B21A38F536BA, ped)) {
        invoke<void>(0xCEA04D83135264CC, ped, 100); // SET_PED_ARMOUR
        PlayFrontendSound("CONFIRM_BEEP");
        UI::ShowToast("Speler: Body Armor 100% aangevuld", ImVec4(0.2f, 0.8f, 1.0f, 1.0f));
        LOG_INFO("Player replenished with full body armor.");
    }
}

void Cheats::CleanPlayerDamage() {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (invoke<BOOL>(0x7239B21A38F536BA, ped)) {
        invoke<void>(0x8FE22675A5A45817, ped); // CLEAR_PED_BLOOD_DAMAGE
        invoke<void>(0x3AC1F7B898F30C05, ped); // RESET_PED_VISIBLE_DAMAGE
        PlayFrontendSound("CONFIRM_BEEP");
        UI::ShowToast("Kleding: Bloed & schade gereinigd", ImVec4(0.6f, 0.9f, 0.9f, 1.0f));
        LOG_INFO("Player visible blood and clothing damage cleaned.");
    }
}

void Cheats::RefillSpecialAbility() {
    Player player = invoke<Player>(0x4F8644AF03D0E0D6);
    invoke<void>(0x3DACA8DDC6FD4980, player, TRUE); // SPECIAL_ABILITY_FILL_METER
    PlayFrontendSound("CONFIRM_BEEP");
    UI::ShowToast("Special Ability: Meter 100% gevuld", ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
    LOG_INFO("Special ability meter refilled.");
}

void Cheats::SuicidePlayer() {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (invoke<BOOL>(0x7239B21A38F536BA, ped)) {
        invoke<void>(0x6B76DC1F3AE6E6A3, ped, 0, 0, 0); // SET_ENTITY_HEALTH(0)
        UI::ShowToast("Speler: Zelfmoord / Respawn getriggerd", ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        LOG_INFO("Player suicide triggered.");
    }
}

void Cheats::RagdollPlayer() {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (invoke<BOOL>(0x7239B21A38F536BA, ped)) {
        invoke<void>(0xAE99FB955581844A, ped, 4000, 4000, 0, FALSE, FALSE, FALSE); // SET_PED_TO_RAGDOLL
        UI::ShowToast("Speler: Ragdoll geactiveerd", ImVec4(1.0f, 0.6f, 0.2f, 1.0f));
        LOG_INFO("Player set to ragdoll.");
    }
}

void Cheats::SetWantedLevel(int level) {
    Player player = invoke<Player>(0x4F8644AF03D0E0D6);
    invoke<void>(0x39FF19C64EF7DA5B, player, level, FALSE);
    invoke<void>(0xE0A7D1E497FFCD6F, player, FALSE);
    PlayFrontendSound("CONFIRM_BEEP");
    char toast[64];
    snprintf(toast, sizeof(toast), "Wanted Level: %d sterren ingesteld", level);
    UI::ShowToast(toast, ImVec4(1.0f, 0.7f, 0.2f, 1.0f));
    LOG_INFO("Wanted level set to %d stars.", level);
}

void Cheats::ClearWantedLevel() {
    Player player = invoke<Player>(0x4F8644AF03D0E0D6);
    invoke<void>(0xB302540597885499, player); // CLEAR_PLAYER_WANTED_LEVEL
    invoke<void>(0x39FF19C64EF7DA5B, player, 0, FALSE);
    PlayFrontendSound("CONFIRM_BEEP");
    UI::ShowToast("Politie: Wanted Level volledig gewist", ImVec4(0.3f, 0.9f, 0.3f, 1.0f));
    LOG_INFO("Wanted level cleared.");
}

void Cheats::AddCash(int amount) {
    const char* statNames[] = { "SP0_TOTAL_CASH", "SP1_TOTAL_CASH", "SP2_TOTAL_CASH" };
    for (const char* name : statNames) {
        Hash h = invoke<Hash>(0xD24D37CC275948CC, name); // GET_HASH_KEY
        int current = 0;
        invoke<BOOL>(0x767FBC2AC802EF3D, h, &current, -1); // STAT_GET_INT
        int nextVal = current + amount;
        if (nextVal < 0 || nextVal > 2000000000) nextVal = 2000000000;
        invoke<BOOL>(0xB3271D7AB655B441, h, nextVal, TRUE); // STAT_SET_INT
    }
    PlayFrontendSound("LOCAL_PLYR_CASH_COUNTER_COMPLETE", "DLC_HEISTS_GENERAL_FRONTEND_SOUNDS");
    char toast[64];
    snprintf(toast, sizeof(toast), "+$%d Cash toegevoegd aan rekening", amount);
    UI::ShowToast(toast, ImVec4(0.2f, 1.0f, 0.4f, 1.0f));
    LOG_INFO("Added $%d cash to story mode characters.", amount);
}

void Cheats::SetMaxCash() {
    const char* statNames[] = { "SP0_TOTAL_CASH", "SP1_TOTAL_CASH", "SP2_TOTAL_CASH" };
    for (const char* name : statNames) {
        Hash h = invoke<Hash>(0xD24D37CC275948CC, name);
        invoke<BOOL>(0xB3271D7AB655B441, h, 2000000000, TRUE); // STAT_SET_INT ($2,000,000,000)
    }
    PlayFrontendSound("LOCAL_PLYR_CASH_COUNTER_COMPLETE", "DLC_HEISTS_GENERAL_FRONTEND_SOUNDS");
    UI::ShowToast("Cash: Maximaal $2.000.000.000 toegekend!", ImVec4(1.0f, 0.85f, 0.0f, 1.0f));
    LOG_INFO("Max cash ($2,000,000,000) granted.");
}

void Cheats::MaxTuneVehicle() {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (!invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) {
        UI::ShowToast("Stap eerst in een voertuig om te tunen!", ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
        return;
    }
    Vehicle veh = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
    if (!veh) return;

    invoke<void>(0x1F2AA07F00B3217A, veh, 0); // SET_VEHICLE_MOD_KIT
    invoke<void>(0x6AF0636DDEDCB6DD, veh, 11, 3, FALSE); // Engine Level 4
    invoke<void>(0x6AF0636DDEDCB6DD, veh, 12, 2, FALSE); // Race Brakes
    invoke<void>(0x6AF0636DDEDCB6DD, veh, 13, 2, FALSE); // Race Transmission
    invoke<void>(0x6AF0636DDEDCB6DD, veh, 15, 3, FALSE); // Competition Suspension
    invoke<void>(0x6AF0636DDEDCB6DD, veh, 16, 4, FALSE); // 100% Armor
    invoke<void>(0x2A1F4F37F95BAD08, veh, 18, TRUE);     // Turbo Tuning
    invoke<void>(0x2A1F4F37F95BAD08, veh, 22, TRUE);     // Xenon Headlights
    invoke<void>(0xEB9DC3C7D8596C46, veh, FALSE);        // Bulletproof Tyres (Can burst = FALSE)
    invoke<void>(0x115722B1B9C14C1C, veh);               // SET_VEHICLE_FIXED
    invoke<void>(0x79D3B596FE44EE8B, veh, 0.0f);         // SET_VEHICLE_DIRT_LEVEL

    {
        std::lock_guard<std::mutex> lock(g_VehicleModCache.mtx);
        if (g_VehicleModCache.hasVehicle) {
            g_VehicleModCache.mods[11].current = 3;
            g_VehicleModCache.mods[12].current = 2;
            g_VehicleModCache.mods[13].current = 2;
            g_VehicleModCache.mods[15].current = 3;
            g_VehicleModCache.mods[16].current = 4;
        }
    }

    PlayFrontendSound("CONFIRM_BEEP");

    UI::ShowToast("Voertuig: Max Performance Tuning toegepast!", ImVec4(0.2f, 0.9f, 0.4f, 1.0f));
    LOG_INFO("Applied max performance modifications to vehicle.");
}

void Cheats::SpawnVehicle(const char* modelName, bool maxTuned) {
    Hash model = invoke<Hash>(0xD24D37CC275948CC, modelName); // GET_HASH_KEY
    invoke<void>(0x963D27A58DF860AC, model); // REQUEST_MODEL

    DWORD start = GetTickCount();
    while (!invoke<BOOL>(0x98A4EB5D89A0C952, model) && (GetTickCount() - start < 1500)) {
        ScriptHookSDK::scriptWait(0);
    }

    if (invoke<BOOL>(0x98A4EB5D89A0C952, model)) {
        Ped ped = invoke<Ped>(0xD80958FC74E988A6);
        Vector3 coords = invoke<Vector3>(0x3FEF770D40960D5A, ped, TRUE);
        float heading = invoke<float>(0xE83D4F9BA2A38914, ped);

        // Safely remove previous vehicle if currently driving
        if (invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) {
            Vehicle oldVeh = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
            if (oldVeh) {
                invoke<void>(0xAD738C3085FE7E11, oldVeh, TRUE, TRUE); // SET_ENTITY_AS_MISSION_ENTITY
                invoke<void>(0xEA386986E786A54F, &oldVeh); // DELETE_VEHICLE
            }
        }

        float rad = heading * 0.0174532925f;
        float spawnX = coords.x - sinf(rad) * 3.0f;
        float spawnY = coords.y + cosf(rad) * 3.0f;

        Vehicle veh = invoke<Vehicle>(0xAF35D0D2583051B0, model, spawnX, spawnY, coords.z + 0.5f, heading, TRUE, FALSE);
        if (veh) {
            invoke<void>(0x49733E92263139D1, veh); // SET_VEHICLE_ON_GROUND_PROPERLY
            invoke<void>(0xF75B0D629E1C063D, ped, veh, -1); // SET_PED_INTO_VEHICLE
            invoke<void>(0x2497C4717C8B881E, veh, TRUE, TRUE, TRUE); // SET_VEHICLE_ENGINE_ON
            invoke<void>(0xE532F5D78798DAAB, model); // SET_MODEL_AS_NO_LONGER_NEEDED

            if (maxTuned || g_Config.spawnMaxTuned) {
                MaxTuneVehicle();
            }

            PlayFrontendSound("CONFIRM_BEEP");
            char toast[96];
            snprintf(toast, sizeof(toast), "Voertuig: %s succesvol gespawned", modelName);
            UI::ShowToast(toast, ImVec4(0.2f, 1.0f, 0.5f, 1.0f));
            LOG_INFO("Spawned vehicle %s into driver seat.", modelName);
        }
    } else {
        char toast[96];
        snprintf(toast, sizeof(toast), "Fout: Model %s kon niet geladen worden", modelName);
        UI::ShowToast(toast, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        LOG_WARN("Failed to load vehicle model: %s", modelName);
    }
}

void Cheats::RepairVehicle() {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) {
        Vehicle veh = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
        if (veh) {
            invoke<void>(0x115722B1B9C14C1C, veh); // SET_VEHICLE_FIXED
            invoke<void>(0x953DA1E1B12C0491, veh); // SET_VEHICLE_DEFORMATION_FIXED
            invoke<void>(0x79D3B596FE44EE8B, veh, 0.0f); // SET_VEHICLE_DIRT_LEVEL
            PlayFrontendSound("CONFIRM_BEEP");
            UI::ShowToast("Voertuig: Volledig gerepareerd en gereinigd", ImVec4(0.2f, 0.9f, 0.4f, 1.0f));
            LOG_INFO("Vehicle repaired and restored to pristine condition.");
        }
    } else {
        UI::ShowToast("Stap eerst in een voertuig!", ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
    }
}

void Cheats::CleanVehicle() {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) {
        Vehicle veh = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
        if (veh) {
            invoke<void>(0x79D3B596FE44EE8B, veh, 0.0f);
            PlayFrontendSound("CONFIRM_BEEP");
            UI::ShowToast("Voertuig: Schoon gewassen", ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
            LOG_INFO("Vehicle cleaned.");
        }
    }
}

void Cheats::BoostVehicle(float force) {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) {
        Vehicle veh = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
        if (veh) {
            float speed = invoke<float>(0xD5037BA82E12416F, veh);
            invoke<void>(0xAB54A438726D25D5, veh, speed + force); // SET_VEHICLE_FORWARD_SPEED
            PlayFrontendSound("RACE_PLACED", "HUD_AWARDS");
            char toast[64];
            snprintf(toast, sizeof(toast), "Rocket Boost: +%.0f m/s geactiveerd!", force);
            UI::ShowToast(toast, ImVec4(1.0f, 0.6f, 0.1f, 1.0f));
            LOG_INFO("Applied rocket speed boost (+%.1f m/s).", force);
        }
    }
}

void Cheats::InstantBrake() {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) {
        Vehicle veh = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
        if (veh) {
            invoke<void>(0xAB54A438726D25D5, veh, 0.0f); // SET_VEHICLE_FORWARD_SPEED(0)
            PlayFrontendSound("CONFIRM_BEEP");
            UI::ShowToast("Noodstop: Voertuig direct stilgezet", ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            LOG_INFO("Vehicle instant brake applied.");
        }
    }
}

void Cheats::AutoFlipVehicle() {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) {
        Vehicle veh = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
        if (veh) {
            invoke<void>(0x49733E92263139D1, veh); // SET_VEHICLE_ON_GROUND_PROPERLY
            PlayFrontendSound("CONFIRM_BEEP");
            UI::ShowToast("Voertuig: Weer rechtop op de wielen gezet", ImVec4(0.2f, 0.9f, 0.4f, 1.0f));
            LOG_INFO("Vehicle flipped upright.");
        }
    }
}

void Cheats::SetLicensePlate(const char* plate) {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) {
        Vehicle veh = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
        if (veh && plate) {
            invoke<void>(0x95A88F0B409CDA47, veh, plate); // SET_VEHICLE_NUMBER_PLATE_TEXT
            PlayFrontendSound("CONFIRM_BEEP");
            char toast[64];
            snprintf(toast, sizeof(toast), "Kenteken gewijzigd naar: %s", plate);
            UI::ShowToast(toast, ImVec4(0.2f, 0.85f, 1.0f, 1.0f));
            LOG_INFO("Vehicle license plate set to: %s", plate);
        }
    }
}

int Cheats::GetVehicleModCount(int modType) {
    if (modType < 0 || modType >= 50) return 0;
    std::lock_guard<std::mutex> lock(g_VehicleModCache.mtx);
    if (!g_VehicleModCache.hasVehicle) return 0;
    return g_VehicleModCache.mods[modType].count;
}

int Cheats::GetVehicleMod(int modType) {
    if (modType < 0 || modType >= 50) return -1;
    std::lock_guard<std::mutex> lock(g_VehicleModCache.mtx);
    if (!g_VehicleModCache.hasVehicle) return -1;
    return g_VehicleModCache.mods[modType].current;
}

void Cheats::SetVehicleMod(int modType, int modIndex) {
    if (modType < 0 || modType >= 50) return;
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (!invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) return;
    Vehicle veh = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
    if (!veh) return;
    invoke<void>(0x1F2AA07F00B3217A, veh, 0); // SET_VEHICLE_MOD_KIT
    invoke<void>(0x6AF0636DDEDCB6DD, veh, modType, modIndex, FALSE); // SET_VEHICLE_MOD
    if (modType >= 0 && modType < 50) {
        std::lock_guard<std::mutex> lock(g_VehicleModCache.mtx);
        g_VehicleModCache.mods[modType].current = modIndex;
    }
    PlayFrontendSound("NAV_UP_DOWN");
}

void Cheats::SetVehicleCustomColors(float pR, float pG, float pB, float sR, float sG, float sB) {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (!invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) return;
    Vehicle veh = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
    if (!veh) return;
    int pr = (int)(pR * 255.0f);
    int pg = (int)(pG * 255.0f);
    int pb = (int)(pB * 255.0f);
    int sr = (int)(sR * 255.0f);
    int sg = (int)(sG * 255.0f);
    int sb = (int)(sB * 255.0f);
    invoke<void>(0x4F1D4BE3A7F24601, veh, pr, pg, pb); // SET_VEHICLE_CUSTOM_PRIMARY_COLOUR
    invoke<void>(0x36CED73BFED89754, veh, sr, sg, sb); // SET_VEHICLE_CUSTOM_SECONDARY_COLOUR
}

void Cheats::SetVehicleNeonState(bool left, bool right, bool front, bool back, float r, float g, float b) {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (!invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) return;
    Vehicle veh = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
    if (!veh) return;
    invoke<void>(0x2AA720E4287BF269, veh, 0, left ? TRUE : FALSE);
    invoke<void>(0x2AA720E4287BF269, veh, 1, right ? TRUE : FALSE);
    invoke<void>(0x2AA720E4287BF269, veh, 2, front ? TRUE : FALSE);
    invoke<void>(0x2AA720E4287BF269, veh, 3, back ? TRUE : FALSE);
    invoke<void>(0x8E0A582209A62695, veh, (int)(r * 255.0f), (int)(g * 255.0f), (int)(b * 255.0f)); // SET_VEHICLE_NEON_COLOUR
}

void Cheats::SetVehicleXenonColor(int colorIndex) {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (!invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) return;
    Vehicle veh = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
    if (!veh) return;
    invoke<void>(0x1F2AA07F00B3217A, veh, 0);
    invoke<void>(0x2A1F4F37F95BAD08, veh, 22, TRUE); // TOGGLE_VEHICLE_MOD(22 = Xenon)
    invoke<void>(0xE41033B25D003A07, veh, colorIndex); // SET_VEHICLE_XENON_LIGHTS_COLOR
    PlayFrontendSound("CONFIRM_BEEP");
}

void Cheats::SetVehicleWindowTint(int tintLevel) {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (!invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) return;
    Vehicle veh = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
    if (!veh) return;
    invoke<void>(0x57C51E6BAD752696, veh, tintLevel); // SET_VEHICLE_WINDOW_TINT
    PlayFrontendSound("CONFIRM_BEEP");
}

void Cheats::SetVehicleWheelType(int wheelType, int wheelIndex) {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (!invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) return;
    Vehicle veh = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
    if (!veh) return;
    invoke<void>(0x1F2AA07F00B3217A, veh, 0);
    invoke<void>(0x487EB21CC7295BA1, veh, wheelType); // SET_VEHICLE_WHEEL_TYPE
    invoke<void>(0x6AF0636DDEDCB6DD, veh, 23, wheelIndex, FALSE); // SET_VEHICLE_MOD(23 = Front Wheels)
    PlayFrontendSound("NAV_UP_DOWN");
}

void Cheats::SetVehicleTyreSmokeColor(float r, float g, float b) {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (!invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) return;
    Vehicle veh = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
    if (!veh) return;
    invoke<void>(0x1F2AA07F00B3217A, veh, 0);
    invoke<void>(0x2A1F4F37F95BAD08, veh, 20, TRUE); // TOGGLE_VEHICLE_MOD(20 = Tyre Smoke)
    invoke<void>(0xB5BA80F839791C0F, veh, (int)(r * 255.0f), (int)(g * 255.0f), (int)(b * 255.0f)); // SET_VEHICLE_TYRE_SMOKE_COLOR
}

void Cheats::ToggleVehicleExtraMod(int modType, bool toggle) {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (!invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) return;
    Vehicle veh = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
    if (!veh) return;
    invoke<void>(0x1F2AA07F00B3217A, veh, 0);
    invoke<void>(0x2A1F4F37F95BAD08, veh, modType, toggle ? TRUE : FALSE);
}

static void GiveWeaponList(const std::vector<const char*>& weapons, const char* categoryName) {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    if (!invoke<BOOL>(0x7239B21A38F536BA, ped)) return;

    for (const char* w : weapons) {
        Hash wHash = invoke<Hash>(0xD24D37CC275948CC, w);
        invoke<void>(0xBF0FD6E56C964FCB, ped, wHash, 9999, FALSE, FALSE); // GIVE_DELAYED_WEAPON_TO_PED
    }
    Cheats::PlayFrontendSound("PICK_UP_WEAPON", "HUD_FRONTEND_CUSTOM_SOUNDSET");
    char toast[64];
    snprintf(toast, sizeof(toast), "Wapens: %s toegekend (Max Ammo)", categoryName);
    UI::ShowToast(toast, ImVec4(1.0f, 0.85f, 0.2f, 1.0f));
}

void Cheats::GiveHandguns() {
    std::vector<const char*> wp = {
        "WEAPON_PISTOL", "WEAPON_COMBATPISTOL", "WEAPON_APPISTOL", "WEAPON_PISTOL50",
        "WEAPON_REVOLVER", "WEAPON_SNSPISTOL", "WEAPON_HEAVYPISTOL", "WEAPON_VINTAGEPISTOL",
        "WEAPON_MARKSMANPISTOL", "WEAPON_DOUBLEACTION"
    };
    GiveWeaponList(wp, "Pistolen & Handvuurwapens");
}

void Cheats::GiveRiflesAndSMGs() {
    std::vector<const char*> wp = {
        "WEAPON_MICROSMG", "WEAPON_SMG", "WEAPON_ASSAULTSMG", "WEAPON_COMBATPDW",
        "WEAPON_MACHINEPISTOL", "WEAPON_MINISMG", "WEAPON_ASSAULTRIFLE", "WEAPON_CARBINERIFLE",
        "WEAPON_ADVANCEDRIFLE", "WEAPON_SPECIALCARBINE", "WEAPON_BULLPUPRIFLE", "WEAPON_COMPACTRIFLE",
        "WEAPON_MG", "WEAPON_COMBATMG", "WEAPON_GUSENBERG"
    };
    GiveWeaponList(wp, "Aanvalsgeweren & SMG's");
}

void Cheats::GiveShotgunsAndSnipers() {
    std::vector<const char*> wp = {
        "WEAPON_PUMPSHOTGUN", "WEAPON_SAWNOFFSHOTGUN", "WEAPON_BULLPUPSHOTGUN", "WEAPON_ASSAULTSHOTGUN",
        "WEAPON_MUSKET", "WEAPON_HEAVYSHOTGUN", "WEAPON_DBSHOTGUN", "WEAPON_AUTOSHOTGUN",
        "WEAPON_SNIPERRIFLE", "WEAPON_HEAVYSNIPER", "WEAPON_MARKSMANRIFLE"
    };
    GiveWeaponList(wp, "Shotguns & Scherpschuttersgeweren");
}

void Cheats::GiveHeavyWeapons() {
    std::vector<const char*> wp = {
        "WEAPON_RPG", "WEAPON_GRENADELAUNCHER", "WEAPON_MINIGUN", "WEAPON_FIREWORK",
        "WEAPON_RAILGUN", "WEAPON_HOMINGLAUNCHER", "WEAPON_COMPACTLAUNCHER", "WEAPON_GRENADE",
        "WEAPON_STICKYBOMB", "WEAPON_PROXMINE", "WEAPON_PIPEBOMB", "WEAPON_MOLOTOV"
    };
    GiveWeaponList(wp, "Zware & Explosieve Wapens");
}

void Cheats::GiveMeleeWeapons() {
    std::vector<const char*> wp = {
        "WEAPON_KNIFE", "WEAPON_NIGHTSTICK", "WEAPON_HAMMER", "WEAPON_BAT", "WEAPON_GOLFCLUB",
        "WEAPON_CROWBAR", "WEAPON_BOTTLE", "WEAPON_DAGGER", "WEAPON_HATCHET", "WEAPON_KNUCKLE",
        "WEAPON_MACHETE", "WEAPON_FLASHLIGHT", "WEAPON_SWITCHBLADE", "WEAPON_POOLCUE", "WEAPON_WRENCH", "WEAPON_BATTLEAXE"
    };
    GiveWeaponList(wp, "Slag- & Steekwapens");
}

void Cheats::GiveAllWeapons() {
    GiveHandguns();
    GiveRiflesAndSMGs();
    GiveShotgunsAndSnipers();
    GiveHeavyWeapons();
    GiveMeleeWeapons();
    UI::ShowToast("Volledig Arsenaal: Alle wapens toegekend!", ImVec4(1.0f, 0.85f, 0.0f, 1.0f));
    LOG_INFO("Given all weapons with max ammunition.");
}

void Cheats::TeleportToCoords(float x, float y, float z) {
    Ped ped = invoke<Ped>(0xD80958FC74E988A6);
    Entity target = ped;
    if (invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) {
        target = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
    }

    if (invoke<BOOL>(0x7239B21A38F536BA, target)) {
        // Freeze entity during collision streaming to prevent falling into the void
        invoke<void>(0x428CA6DBD1094446, target, TRUE); // FREEZE_ENTITY_POSITION
        invoke<void>(0x07503F7948F491A7, x, y, z); // REQUEST_COLLISION_AT_COORD
        invoke<void>(0x4448EB75B4904BDB, x, y, z, 100.0f, 0); // LOAD_SCENE
        invoke<void>(0x06843DA7060A026B, target, x, y, z, FALSE, FALSE, FALSE, TRUE); // SET_ENTITY_COORDS

        ScriptHookSDK::scriptWait(50);
        invoke<void>(0x428CA6DBD1094446, target, FALSE); // Unfreeze
        PlayFrontendSound("CONFIRM_BEEP");
        char toast[64];
        snprintf(toast, sizeof(toast), "Geteleporteerd naar (%.0f, %.0f, %.0f)", x, y, z);
        UI::ShowToast(toast, ImVec4(0.2f, 0.85f, 1.0f, 1.0f));
        LOG_INFO("Teleported to (%.1f, %.1f, %.1f).", x, y, z);
    }
}

void Cheats::TeleportToWaypoint() {
    int blip = invoke<int>(0x1BEDE233E6CD2A1F, 8); // GET_FIRST_BLIP_INFO_ID (8 = Waypoint)
    if (blip && invoke<BOOL>(0x1DD1F58F493F1DA5)) { // IS_WAYPOINT_ACTIVE
        Vector3 coords = invoke<Vector3>(0xFA7C7F0AADF25D09, blip); // GET_BLIP_INFO_ID_COORD

        Ped ped = invoke<Ped>(0xD80958FC74E988A6);
        Entity target = ped;
        if (invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) {
            target = invoke<Vehicle>(0x9A9112A0FE9A4713, ped, FALSE);
        }

        invoke<void>(0x428CA6DBD1094446, target, TRUE); // FREEZE_ENTITY_POSITION
        invoke<void>(0x07503F7948F491A7, coords.x, coords.y, 100.0f); // REQUEST_COLLISION_AT_COORD
        invoke<void>(0x4448EB75B4904BDB, coords.x, coords.y, 100.0f, 150.0f, 0); // LOAD_SCENE

        float groundZ = 0.0f;
        BOOL groundFound = FALSE;
        for (float testZ = 0.0f; testZ < 850.0f; testZ += 30.0f) {
            groundFound = invoke<BOOL>(0xC906A7DAB05C8D2B, coords.x, coords.y, testZ, &groundZ, FALSE);
            if (groundFound) break;
        }

        float finalZ = groundFound ? (groundZ + 1.2f) : 100.0f;
        invoke<void>(0x06843DA7060A026B, target, coords.x, coords.y, finalZ, FALSE, FALSE, FALSE, TRUE);

        ScriptHookSDK::scriptWait(50);
        invoke<void>(0x428CA6DBD1094446, target, FALSE);

        PlayFrontendSound("CONFIRM_BEEP");
        UI::ShowToast("Succesvol geteleporteerd naar Waypoint", ImVec4(0.2f, 1.0f, 0.4f, 1.0f));
        LOG_INFO("Teleported safely to map Waypoint.");
    } else {
        PlayFrontendSound("ERROR", "HUD_FRONTEND_DEFAULT_SOUNDSET");
        UI::ShowToast("Geen actieve Waypoint gemarkeerd op de kaart!", ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        LOG_WARN("No active map Waypoint found.");
    }
}

void Cheats::SetWeather(const char* weatherType) {
    invoke<void>(0xED712CA327900C8A, weatherType); // SET_WEATHER_TYPE_NOW_PERSIST
    PlayFrontendSound("CONFIRM_BEEP");
    char toast[64];
    snprintf(toast, sizeof(toast), "Weer gewijzigd naar: %s", weatherType);
    UI::ShowToast(toast, ImVec4(0.4f, 0.85f, 1.0f, 1.0f));
    LOG_INFO("Weather set to %s.", weatherType);
}

void Cheats::SetTime(int hour, int minute) {
    invoke<void>(0x47C3B5848C3E45D8, hour, minute, 0); // SET_CLOCK_TIME
    char toast[64];
    snprintf(toast, sizeof(toast), "Klok ingesteld op: %02d:%02d", hour, minute);
    UI::ShowToast(toast, ImVec4(1.0f, 0.85f, 0.2f, 1.0f));
    LOG_INFO("Clock set to %02d:%02d.", hour, minute);
}

void Cheats::SetGravity(int level) {
    SetGravityLevel(level);
}

void Cheats::SetGravityLevel(int level) {
    if (level < 0) level = 0;
    if (level > 3) level = 3;
    g_Config.worldGravityLevel = level;
    invoke<void>(0x740E14FAD5842351, level); // SET_GRAVITY_LEVEL (0=Earth, 1=Moon, 2=Very Low, 3=Zero)
    const char* gNames[] = { "Aardse zwaartekracht (Normaal)", "Maan zwaartekracht (Laag)", "Zeer lage zwaartekracht", "Nul zwaartekracht (Zweven)" };
    UI::ShowToast(gNames[level], ImVec4(0.5f, 0.8f, 1.0f, 1.0f));
    LOG_INFO("Gravity level set to %d.", level);
}

void Cheats::SetTimeScale(float scale) {
    if (scale < 0.05f) scale = 0.05f;
    if (scale > 1.0f) scale = 1.0f;
    g_Config.worldMatrixTimeScale = scale;
    invoke<void>(0x1D408577D440E81E, scale); // SET_TIME_SCALE
    char toast[64];
    snprintf(toast, sizeof(toast), "Matrix Tijd: %.1fx snelheid", scale);
    UI::ShowToast(toast, ImVec4(0.2f, 1.0f, 0.5f, 1.0f));
    LOG_INFO("Time scale set to %.2f.", scale);
}

void Cheats::SetBlackout(bool enable) {
    g_Config.worldBlackout = enable;
    invoke<void>(0x1268615ACE24D504, enable ? TRUE : FALSE); // SET_ARTIFICIAL_LIGHTS_STATE
    UI::ShowToast(enable ? "Blackout: Alle stadsverlichting uitgeschakeld" : "Blackout: Stadsverlichting ingeschakeld", ImVec4(0.7f, 0.7f, 1.0f, 1.0f));
    LOG_INFO("Blackout mode set to %s.", enable ? "ENABLED" : "DISABLED");
}

void Cheats::SetInGameControlsDisabled(bool disabled) {
    if (disabled) {
        // Suspend player movement, weapons, and camera look in GTA V
        invoke<void>(0x5F4B6931816E599B, 0); // DISABLE_ALL_CONTROL_ACTIONS(0)
        invoke<void>(0x5F4B6931816E599B, 1); // DISABLE_ALL_CONTROL_ACTIONS(1)
        invoke<void>(0x5F4B6931816E599B, 2); // DISABLE_ALL_CONTROL_ACTIONS(2)
        invoke<void>(0xAAE7CE1D63167423);   // HUD::SET_MOUSE_CURSOR_THIS_FRAME
    }
}

void Cheats::ProcessControllerInput() {
    if (!g_Config.controllerEnabled) return;

    // 1. When Menu is CLOSED:
    if (!UI::s_MenuOpen) {
        // Check RB + D-Pad Right to toggle menu open
        // 21 = INPUT_SPRINT (RB on foot), 76 = INPUT_VEH_HANDBRAKE (RB in vehicle)
        // 175 = INPUT_PHONE_RIGHT (D-Pad Right)
        bool rbPressed = invoke<BOOL>(0xF3A21BCD95725A4A, 0, 21) || invoke<BOOL>(0xF3A21BCD95725A4A, 0, 76) ||
                         invoke<BOOL>(0xE2587F8CBBD87B1D, 0, 21) || invoke<BOOL>(0xE2587F8CBBD87B1D, 0, 76);
        bool dpadRight = invoke<BOOL>(0x580417101DDB492F, 0, 175) || invoke<BOOL>(0x91AEF906BCA88877, 0, 175);

        if (rbPressed && dpadRight) {
            UI::s_MenuOpen = true;
            PlayFrontendSound("SELECT", "HUD_FRONTEND_DEFAULT_SOUNDSET");
            UI::ShowToast("Mod Menu Geopend via Controller [RB + D-Pad]", ImVec4(0.2f, 0.9f, 0.4f, 1.0f));
            LOG_INFO("Menu opened via Controller (RB + D-Pad Right).");
            return;
        }

        // Driving Shortcuts
        if (g_Config.controllerDrivingShortcuts) {
            Ped ped = invoke<Ped>(0xD80958FC74E988A6);
            if (invoke<BOOL>(0x997ABD671D25CA0B, ped, FALSE)) { // In any vehicle
                // Horn Boost (86 = INPUT_VEH_HORN / L3)
                if (invoke<BOOL>(0x580417101DDB492F, 0, 86) || invoke<BOOL>(0x91AEF906BCA88877, 0, 86)) {
                    BoostVehicle(30.0f);
                }

                // Quick Repair (LB + D-Pad Down)
                // 19 = INPUT_CHARACTER_WHEEL (LB), 173 = INPUT_PHONE_DOWN (D-Pad Down)
                bool lbPressed = invoke<BOOL>(0xF3A21BCD95725A4A, 0, 19) || invoke<BOOL>(0xE2587F8CBBD87B1D, 0, 19);
                bool dpadDown = invoke<BOOL>(0x580417101DDB492F, 0, 173) || invoke<BOOL>(0x91AEF906BCA88877, 0, 173);
                if (lbPressed && dpadDown) {
                    RepairVehicle();
                }
            }
        }
    }
    // 2. When Menu is OPEN:
    else {
        // Suspend in-game controls continuously
        SetInGameControlsDisabled(true);

        // Close menu via B / Cancel button (202 = INPUT_FRONTEND_CANCEL)
        if (invoke<BOOL>(0x580417101DDB492F, 0, 202) || invoke<BOOL>(0x91AEF906BCA88877, 0, 202)) {
            UI::s_MenuOpen = false;
            PlayFrontendSound("BACK", "HUD_FRONTEND_DEFAULT_SOUNDSET");
            LOG_INFO("Menu closed via Controller B button.");
            return;
        }

        // Close menu via RB + D-Pad Right
        bool rbPressed = invoke<BOOL>(0xF3A21BCD95725A4A, 0, 21) || invoke<BOOL>(0xF3A21BCD95725A4A, 0, 76) ||
                         invoke<BOOL>(0xE2587F8CBBD87B1D, 0, 21) || invoke<BOOL>(0xE2587F8CBBD87B1D, 0, 76);
        bool dpadRight = invoke<BOOL>(0x580417101DDB492F, 0, 175) || invoke<BOOL>(0x91AEF906BCA88877, 0, 175);
        if (rbPressed && dpadRight) {
            UI::s_MenuOpen = false;
            PlayFrontendSound("BACK", "HUD_FRONTEND_DEFAULT_SOUNDSET");
            LOG_INFO("Menu closed via Controller RB + D-Pad Right.");
            return;
        }

        // Tab Navigation via Bumpers (LB = 205, RB = 206)
        if (invoke<BOOL>(0x580417101DDB492F, 0, 206) || invoke<BOOL>(0x91AEF906BCA88877, 0, 206)) {
            UI::CycleTab(1); // Next tab
            PlayFrontendSound("NAV_LEFT_RIGHT", "HUD_FRONTEND_DEFAULT_SOUNDSET");
        } else if (invoke<BOOL>(0x580417101DDB492F, 0, 205) || invoke<BOOL>(0x91AEF906BCA88877, 0, 205)) {
            UI::CycleTab(-1); // Prev tab
            PlayFrontendSound("NAV_LEFT_RIGHT", "HUD_FRONTEND_DEFAULT_SOUNDSET");
        }

        // Virtual Cursor Navigation via Right Stick (1 = INPUT_LOOK_LR, 2 = INPUT_LOOK_UD)
        float rx = invoke<float>(0x11E65974A982637C, 0, 1);
        float ry = invoke<float>(0x11E65974A982637C, 0, 2);
        // Click using R3 (29 = INPUT_MELEE_GRAPPLE) or A button (201 = INPUT_FRONTEND_ACCEPT)
        bool isClicking = invoke<BOOL>(0xE2587F8CBBD87B1D, 0, 29) || invoke<BOOL>(0xE2587F8CBBD87B1D, 0, 201);

        UI::UpdateVirtualCursor(rx, ry, isClicking);
    }
}
