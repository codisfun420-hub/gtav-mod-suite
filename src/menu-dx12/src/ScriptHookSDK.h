#pragma once
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include "Logger.h"

typedef void (*KeyboardHandler)(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow);
typedef void (*PresentCallback)(void* swapChain);

struct Vector3 {
    float x, _pad1;
    float y, _pad2;
    float z, _pad3;
};

typedef int Entity;
typedef int Player;
typedef int Ped;
typedef int Vehicle;
typedef int Cam;
typedef uint32_t Hash;

class ScriptHookSDK {
public:
    typedef void (*pfnScriptRegister)(HMODULE module, void(*LP_SCRIPT_MAIN)());
    typedef void (*pfnScriptUnregister)(HMODULE module);
    typedef void (*pfnScriptWait)(DWORD time);
    typedef void (*pfnNativeInit)(UINT64 hash);
    typedef void (*pfnNativePush64)(UINT64 val);
    typedef PUINT64 (*pfnNativeCall)();
    typedef UINT64* (*pfnGetGlobalPtr)(int globalId);
    typedef void (*pfnKeyboardHandlerRegister)(KeyboardHandler handler);
    typedef void (*pfnKeyboardHandlerUnregister)(KeyboardHandler handler);
    typedef void (*pfnPresentCallbackRegister)(PresentCallback cb);
    typedef void (*pfnPresentCallbackUnregister)(PresentCallback cb);

    static inline pfnScriptRegister scriptRegister = nullptr;
    static inline pfnScriptUnregister scriptUnregister = nullptr;
    static inline pfnScriptWait scriptWait = nullptr;
    static inline pfnNativeInit nativeInit = nullptr;
    static inline pfnNativePush64 nativePush64 = nullptr;
    static inline pfnNativeCall nativeCall = nullptr;
    static inline pfnGetGlobalPtr getGlobalPtr = nullptr;
    static inline pfnKeyboardHandlerRegister keyboardHandlerRegister = nullptr;
    static inline pfnKeyboardHandlerUnregister keyboardHandlerUnregister = nullptr;
    static inline pfnPresentCallbackRegister presentCallbackRegister = nullptr;
    static inline pfnPresentCallbackUnregister presentCallbackUnregister = nullptr;

    static bool Init(HMODULE hShv = nullptr, const char* gameDir = nullptr) {
        if (!hShv) {
            hShv = GetModuleHandleA("ScriptHookV.dll");
        }
        if (!hShv && gameDir && gameDir[0] != '\0') {
            std::string fullPath = std::string(gameDir) + "\\ScriptHookV.dll";
            hShv = LoadLibraryA(fullPath.c_str());
        }
        if (!hShv) {
            hShv = LoadLibraryA("ScriptHookV.dll");
        }
        if (!hShv) {
            return false;
        }

        #define RESOLVE_SYM(fn, mangled) \
            fn = reinterpret_cast<decltype(fn)>(GetProcAddress(hShv, mangled)); \
            if (!fn) fn = reinterpret_cast<decltype(fn)>(GetProcAddress(hShv, #fn)); \
            if (!fn) LOG_WARN("Could not resolve ScriptHookV export: " #fn " (" mangled ")");

        RESOLVE_SYM(scriptRegister, "?scriptRegister@@YAXPEAUHINSTANCE__@@P6AXXZ@Z");
        RESOLVE_SYM(scriptUnregister, "?scriptUnregister@@YAXPEAUHINSTANCE__@@@Z");
        RESOLVE_SYM(scriptWait, "?scriptWait@@YAXK@Z");
        RESOLVE_SYM(nativeInit, "?nativeInit@@YAX_K@Z");
        RESOLVE_SYM(nativePush64, "?nativePush64@@YAX_K@Z");
        RESOLVE_SYM(nativeCall, "?nativeCall@@YAPEA_KXZ");
        RESOLVE_SYM(getGlobalPtr, "?getGlobalPtr@@YAPEA_KH@Z");
        RESOLVE_SYM(keyboardHandlerRegister, "?keyboardHandlerRegister@@YAXP6AXKGEHHHH@Z@Z");
        RESOLVE_SYM(keyboardHandlerUnregister, "?keyboardHandlerUnregister@@YAXP6AXKGEHHHH@Z@Z");
        RESOLVE_SYM(presentCallbackRegister, "?presentCallbackRegister@@YAXP6AXPEAX@Z@Z");
        RESOLVE_SYM(presentCallbackUnregister, "?presentCallbackUnregister@@YAXP6AXPEAX@Z@Z");

        #undef RESOLVE_SYM

        bool ok = (scriptRegister && scriptWait && nativeInit && nativePush64 && nativeCall);
        if (ok) {
            LOG_INFO("ScriptHookV SDK bindings successfully resolved and initialized.");
        } else {
            LOG_ERROR("Essential ScriptHookV exports missing!");
        }
        return ok;
    }

    template<typename T>
    static void PushArg(T val) {
        UINT64 v = 0;
        if constexpr (sizeof(T) <= sizeof(UINT64)) {
            std::memcpy(&v, &val, sizeof(T));
        } else {
            v = (UINT64)val;
        }
        nativePush64(v);
    }

    template<typename R, typename... Args>
    static R Invoke(UINT64 hash, Args... args) {
        if (!nativeInit || !nativeCall) {
            if constexpr (std::is_void_v<R>) {
                return;
            } else {
                return R{};
            }
        }
        nativeInit(hash);
        (PushArg(args), ...);
        PUINT64 result = nativeCall();
        if constexpr (std::is_void_v<R>) {
            return;
        } else if constexpr (std::is_same_v<R, Vector3>) {
            Vector3* vec = reinterpret_cast<Vector3*>(result);
            return vec ? *vec : Vector3{0, 0, 0, 0, 0, 0};
        } else {
            return result ? *reinterpret_cast<R*>(result) : R{};
        }
    }
};

template<typename R, typename... Args>
inline R invoke(UINT64 hash, Args... args) {
    return ScriptHookSDK::Invoke<R>(hash, args...);
}

inline void invoke_void(UINT64 hash) {
    ScriptHookSDK::Invoke<void>(hash);
}
