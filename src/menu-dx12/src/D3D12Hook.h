#pragma once
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>

class D3D12Hook {
public:
    static bool Init();
    static void Shutdown();

    static inline bool s_Hooked = false;
    static inline HWND s_Hwnd = nullptr;
};
