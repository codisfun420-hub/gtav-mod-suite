@echo off
echo [*] Compiling EnhancedImGuiMenu.asi using zig c++ with SEH and CrashHandler...
zig c++ -target x86_64-windows -std=c++17 -fms-extensions -O2 -shared -I. -I./imgui -I./minhook/include src/main.cpp src/CrashHandler.cpp src/D3D12Hook.cpp src/UI.cpp src/Cheats.cpp src/Config.cpp src/Logger.cpp imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/backends/imgui_impl_win32.cpp imgui/backends/imgui_impl_dx12.cpp -x c++ minhook/src/buffer.c minhook/src/hook.c minhook/src/trampoline.c minhook/src/hde/hde64.c -ldxgi -ld3d12 -ld3dcompiler_47 -ldwmapi -lgdi32 -luser32 -lkernel32 -ldbghelp -o EnhancedImGuiMenu.asi
if %errorlevel% equ 0 (
    echo [OK] EnhancedImGuiMenu.asi compiled successfully.
) else (
    echo [ERR] Compilation failed with error code %errorlevel%.
)
