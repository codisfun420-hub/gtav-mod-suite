#include "D3D12Hook.h"
#include "UI.h"
#include "Config.h"
#include "Logger.h"
#include "minhook/include/MinHook.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx12.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef HRESULT (WINAPI *pfnPresent)(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);
typedef HRESULT (WINAPI *pfnResizeBuffers)(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
typedef void (WINAPI *pfnExecuteCommandLists)(ID3D12CommandQueue* pQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);

static pfnPresent oPresent = nullptr;
static pfnResizeBuffers oResizeBuffers = nullptr;
static pfnExecuteCommandLists oExecuteCommandLists = nullptr;
typedef struct _XINPUT_GAMEPAD_LOCAL {
    WORD wButtons;
    BYTE bLeftTrigger;
    BYTE bRightTrigger;
    SHORT sThumbLX;
    SHORT sThumbLY;
    SHORT sThumbRX;
    SHORT sThumbRY;
} XINPUT_GAMEPAD_LOCAL;

typedef struct _XINPUT_STATE_LOCAL {
    DWORD dwPacketNumber;
    XINPUT_GAMEPAD_LOCAL Gamepad;
} XINPUT_STATE_LOCAL;

typedef DWORD (WINAPI *pfnXInputGetState)(DWORD dwUserIndex, XINPUT_STATE_LOCAL* pState);
static pfnXInputGetState s_pfnXInputGetState = nullptr;
static bool s_XInputAttempted = false;
static WNDPROC oWndProc = nullptr;

struct FrameContext {
    ID3D12CommandAllocator* CommandAllocator = nullptr;
    UINT64 FenceValue = 0;
};

static ID3D12Device* g_pd3dDevice = nullptr;
static ID3D12CommandQueue* g_pCommandQueue = nullptr;
static ID3D12DescriptorHeap* g_pRtvDescHeap = nullptr;
static ID3D12DescriptorHeap* g_pSrvDescHeap = nullptr;
static FrameContext g_FrameContext[8];
static ID3D12GraphicsCommandList* g_pCommandList = nullptr;
static ID3D12Resource* g_pBackBuffers[8] = { nullptr };
static ID3D12Fence* g_pFence = nullptr;
static HANDLE g_hFenceEvent = nullptr;
static UINT64 g_FenceValue = 0;
static UINT g_BufferCount = 0;
static UINT g_RtvDescriptorSize = 0;
static bool g_bImGuiInitialized = false;
static bool s_WndProcIsUnicode = true;
static IDXGISwapChain3* s_pCurrentSwapChain = nullptr;

static LRESULT CALLBACK hk_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) {
        if ((int)wParam == g_Config.toggleKey || (int)wParam == VK_F11) {
            UI::s_MenuOpen = !UI::s_MenuOpen;
            LOG_INFO("Menu toggled via hotkey (key: 0x%X) -> %s", (int)wParam, UI::s_MenuOpen ? "OPEN" : "CLOSED");
            return 0;
        } else if (UI::s_MenuOpen && wParam == VK_ESCAPE) {
            UI::s_MenuOpen = false;
            LOG_INFO("Menu closed via ESC.");
            return 0;
        }
    }

    if (UI::s_MenuOpen) {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

        // Suspend all game mouse input while menu is open so player doesn't attack/fire in-game
        if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) {
            return 1;
        }
        // Suspend all game keyboard/text input while menu is open so typing in search doesn't trigger game actions
        if (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST) {
            return 1;
        }
    }

    if (s_WndProcIsUnicode) {
        return CallWindowProcW(oWndProc, hWnd, uMsg, wParam, lParam);
    } else {
        return CallWindowProcA(oWndProc, hWnd, uMsg, wParam, lParam);
    }
}

static void hk_ExecuteCommandLists(ID3D12CommandQueue* pQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists) {
    if (pQueue) {
        D3D12_COMMAND_QUEUE_DESC desc = pQueue->GetDesc();
        if (desc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
            if (g_pCommandQueue != pQueue) {
                LOG_INFO("Active Direct Command Queue updated: %p -> %p", g_pCommandQueue, pQueue);
                g_pCommandQueue = pQueue;
            }
        }
    }
    if (oExecuteCommandLists) {
        oExecuteCommandLists(pQueue, NumCommandLists, ppCommandLists);
    }
}

static void WaitForLastSubmittedFrame() {
    if (g_pFence && g_hFenceEvent && g_pCommandQueue) {
        g_FenceValue++;
        if (SUCCEEDED(g_pCommandQueue->Signal(g_pFence, g_FenceValue))) {
            if (g_pFence->GetCompletedValue() < g_FenceValue) {
                g_pFence->SetEventOnCompletion(g_FenceValue, g_hFenceEvent);
                WaitForSingleObject(g_hFenceEvent, 2000); // Bounded wait
            }
        }
    }
}

static void CleanupRenderTarget() {
    WaitForLastSubmittedFrame();
    for (UINT i = 0; i < g_BufferCount; ++i) {
        if (g_pBackBuffers[i]) {
            g_pBackBuffers[i]->Release();
            g_pBackBuffers[i] = nullptr;
        }
    }
}

static void CreateRenderTarget(IDXGISwapChain3* pSwapChain) {
    for (UINT i = 0; i < g_BufferCount; ++i) {
        if (SUCCEEDED(pSwapChain->GetBuffer(i, IID_PPV_ARGS(&g_pBackBuffers[i])))) {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
            rtvHandle.ptr += i * g_RtvDescriptorSize;
            g_pd3dDevice->CreateRenderTargetView(g_pBackBuffers[i], nullptr, rtvHandle);
        }
    }
}

static HRESULT WINAPI hk_ResizeBuffers(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
    LOG_INFO("DirectX 12 SwapChain ResizeBuffers triggered (Buffers: %u, Width: %u, Height: %u)", BufferCount, Width, Height);
    if (g_bImGuiInitialized) {
        CleanupRenderTarget();
    }
    if (BufferCount != 0) {
        g_BufferCount = BufferCount > 8 ? 8 : BufferCount;
    }
    HRESULT hr = oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    if (SUCCEEDED(hr) && g_bImGuiInitialized) {
        if (BufferCount == 0 && pSwapChain) {
            DXGI_SWAP_CHAIN_DESC scDesc;
            pSwapChain->GetDesc(&scDesc);
            g_BufferCount = scDesc.BufferCount > 8 ? 8 : scDesc.BufferCount;
        }
        CreateRenderTarget(pSwapChain);
    }
    return hr;
}

static HRESULT WINAPI hk_Present(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!pSwapChain) return oPresent(pSwapChain, SyncInterval, Flags);

    if (s_pCurrentSwapChain != pSwapChain) {
        LOG_INFO("DirectX 12 SwapChain changed (%p -> %p). Recreating render targets...", s_pCurrentSwapChain, pSwapChain);
        if (g_bImGuiInitialized && g_pd3dDevice && g_pRtvDescHeap) {
            CleanupRenderTarget();
            DXGI_SWAP_CHAIN_DESC scDesc;
            pSwapChain->GetDesc(&scDesc);
            g_BufferCount = scDesc.BufferCount > 8 ? 8 : scDesc.BufferCount;
            CreateRenderTarget(pSwapChain);
        }
        s_pCurrentSwapChain = pSwapChain;
    }

    if (!g_bImGuiInitialized) {
        if (FAILED(pSwapChain->GetDevice(IID_PPV_ARGS(&g_pd3dDevice)))) {
            return oPresent(pSwapChain, SyncInterval, Flags);
        }

        // Wait until the game's actual Direct Command Queue is captured
        if (!g_pCommandQueue) {
            return oPresent(pSwapChain, SyncInterval, Flags);
        }

        DXGI_SWAP_CHAIN_DESC desc;
        pSwapChain->GetDesc(&desc);
        D3D12Hook::s_Hwnd = desc.OutputWindow;
        g_BufferCount = desc.BufferCount > 8 ? 8 : desc.BufferCount;
        if (g_BufferCount == 0) g_BufferCount = 2;

        // RTV Heap
        D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
        rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvDesc.NumDescriptors = g_BufferCount;
        rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        if (FAILED(g_pd3dDevice->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&g_pRtvDescHeap)))) {
            LOG_ERROR("Failed to create RTV descriptor heap.");
            return oPresent(pSwapChain, SyncInterval, Flags);
        }
        g_RtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // SRV Heap (for ImGui font texture)
        D3D12_DESCRIPTOR_HEAP_DESC srvDesc = {};
        srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvDesc.NumDescriptors = 1;
        srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (FAILED(g_pd3dDevice->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&g_pSrvDescHeap)))) {
            LOG_ERROR("Failed to create SRV descriptor heap.");
            return oPresent(pSwapChain, SyncInterval, Flags);
        }

        // Allocators & List
        for (UINT i = 0; i < g_BufferCount; ++i) {
            if (FAILED(g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_FrameContext[i].CommandAllocator)))) {
                LOG_ERROR("Failed to create command allocator %u", i);
                return oPresent(pSwapChain, SyncInterval, Flags);
            }
            g_FrameContext[i].FenceValue = 0;
        }
        if (FAILED(g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_FrameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&g_pCommandList)))) {
            LOG_ERROR("Failed to create graphics command list.");
            return oPresent(pSwapChain, SyncInterval, Flags);
        }
        g_pCommandList->Close();

        // Fence & Event for GPU synchronization
        if (FAILED(g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_pFence)))) {
            LOG_ERROR("Failed to create D3D12 fence.");
            return oPresent(pSwapChain, SyncInterval, Flags);
        }
        g_hFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!g_hFenceEvent) {
            LOG_ERROR("Failed to create fence event.");
            return oPresent(pSwapChain, SyncInterval, Flags);
        }
        g_FenceValue = 0;

        CreateRenderTarget(pSwapChain);

        // Hook Window Procedure
        if (D3D12Hook::s_Hwnd) {
            s_WndProcIsUnicode = IsWindowUnicode(D3D12Hook::s_Hwnd) != FALSE;
            if (s_WndProcIsUnicode) {
                oWndProc = (WNDPROC)SetWindowLongPtrW(D3D12Hook::s_Hwnd, GWLP_WNDPROC, (LONG_PTR)hk_WndProc);
            } else {
                oWndProc = (WNDPROC)SetWindowLongPtrA(D3D12Hook::s_Hwnd, GWLP_WNDPROC, (LONG_PTR)hk_WndProc);
            }
            LOG_INFO("Hooked game Window Procedure (HWND: %p, Unicode: %d).", D3D12Hook::s_Hwnd, s_WndProcIsUnicode ? 1 : 0);
        }

        // Init ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr; // Prevent imgui.ini from cluttering game root

        ImGui_ImplWin32_Init(D3D12Hook::s_Hwnd);

        ImGui_ImplDX12_InitInfo initInfo = {};
        initInfo.Device = g_pd3dDevice;
        initInfo.CommandQueue = g_pCommandQueue;
        initInfo.NumFramesInFlight = (int)g_BufferCount;
        initInfo.RTVFormat = desc.BufferDesc.Format;
        initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
        initInfo.SrvDescriptorHeap = g_pSrvDescHeap;
        initInfo.LegacySingleSrvCpuDescriptor = g_pSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
        initInfo.LegacySingleSrvGpuDescriptor = g_pSrvDescHeap->GetGPUDescriptorHandleForHeapStart();
        initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu) {
            *out_cpu = info->SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            *out_gpu = info->SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
        };
        ImGui_ImplDX12_Init(&initInfo);

        UI::InitStyle();
        g_bImGuiInitialized = true;
        LOG_INFO("Dear ImGui DX12 & Win32 backend initialized successfully with fence synchronization.");
    }
 
    // Always poll hotkeys on the render thread if game has foreground focus
    if (g_bImGuiInitialized) {
        HWND fg = GetForegroundWindow();
        DWORD fgPid = 0;
        GetWindowThreadProcessId(fg, &fgPid);
        if (fgPid == GetCurrentProcessId()) {
            static bool s_KeyWasDown = false;
            bool isDown = ((GetAsyncKeyState(g_Config.toggleKey) & 0x8000) != 0) ||
                          ((GetAsyncKeyState(VK_F11) & 0x8000) != 0);
            if (isDown && !s_KeyWasDown) {
                UI::s_MenuOpen = !UI::s_MenuOpen;
                LOG_INFO("Menu toggled via Present GetAsyncKeyState (key: 0x%X) -> %s", g_Config.toggleKey, UI::s_MenuOpen ? "OPEN" : "CLOSED");
            }
            s_KeyWasDown = isDown;

            if (UI::s_MenuOpen) {
                static bool s_EscWasDown = false;
                bool escDown = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
                if (escDown && !s_EscWasDown) {
                    UI::s_MenuOpen = false;
                    LOG_INFO("Menu closed via ESC in Present.");
                }
                s_EscWasDown = escDown;
            }

            // Controller / Gamepad Support via dynamic XInput
            if (g_Config.controllerEnabled) {
                if (!s_XInputAttempted) {
                    HMODULE hXInput = LoadLibraryA("xinput1_4.dll");
                    if (!hXInput) hXInput = LoadLibraryA("xinput1_3.dll");
                    if (!hXInput) hXInput = LoadLibraryA("xinput9_1_0.dll");
                    if (hXInput) {
                        s_pfnXInputGetState = (pfnXInputGetState)GetProcAddress(hXInput, "XInputGetState");
                    }
                    s_XInputAttempted = true;
                }

                if (s_pfnXInputGetState) {
                    XINPUT_STATE_LOCAL state = {};
                    if (s_pfnXInputGetState(0, &state) == ERROR_SUCCESS) {
                        // Check RB + DPad Right to toggle menu (0x0200 = RB, 0x0008 = DPad Right)
                        bool rb = (state.Gamepad.wButtons & 0x0200) != 0;
                        bool dpadR = (state.Gamepad.wButtons & 0x0008) != 0;
                        static bool s_PadComboWasDown = false;
                        bool padCombo = (rb && dpadR);
                        if (padCombo && !s_PadComboWasDown) {
                            UI::s_MenuOpen = !UI::s_MenuOpen;
                            LOG_INFO("Menu toggled via XInput (RB + D-Pad Right) -> %s", UI::s_MenuOpen ? "OPEN" : "CLOSED");
                        }
                        s_PadComboWasDown = padCombo;

                        if (UI::s_MenuOpen) {
                            // B button to close (0x2000 = B)
                            static bool s_PadBWasDown = false;
                            bool padB = (state.Gamepad.wButtons & 0x2000) != 0;
                            if (padB && !s_PadBWasDown) {
                                UI::s_MenuOpen = false;
                                LOG_INFO("Menu closed via XInput B button.");
                            }
                            s_PadBWasDown = padB;

                            // LB / RB tab switching (0x0100 = LB, 0x0200 = RB)
                            static bool s_PadLBWasDown = false;
                            static bool s_PadRBWasDown = false;
                            bool padLB = (state.Gamepad.wButtons & 0x0100) != 0;
                            bool padRB = (state.Gamepad.wButtons & 0x0200) != 0;
                            if (padRB && !s_PadRBWasDown && !dpadR) {
                                UI::CycleTab(1);
                            }
                            if (padLB && !s_PadLBWasDown) {
                                UI::CycleTab(-1);
                            }
                            s_PadLBWasDown = padLB;
                            s_PadRBWasDown = padRB;

                            // Virtual cursor via Right Thumbstick
                            SHORT rx = state.Gamepad.sThumbRX;
                            SHORT ry = state.Gamepad.sThumbRY;
                            float normX = 0.0f, normY = 0.0f;
                            if (abs(rx) > 7000) normX = (float)rx / 32767.0f;
                            if (abs(ry) > 7000) normY = -(float)ry / 32767.0f; // Invert Y for screen coords
                            bool isClick = (state.Gamepad.wButtons & 0x0080) != 0 || (state.Gamepad.wButtons & 0x1000) != 0;
                            UI::UpdateVirtualCursor(normX, normY, isClick);
                        }
                    }
                }
            }
        }
    }

    // Render ImGui when the menu is actively open, when HUD is enabled, or when active toast notifications exist
    if (g_bImGuiInitialized && (UI::s_MenuOpen || g_Config.showTelemetryHud || UI::HasActiveToasts())) {
        __try {
            UINT bufferIdx = pSwapChain->GetCurrentBackBufferIndex();
            if (bufferIdx >= g_BufferCount) bufferIdx = 0;

            FrameContext& frame = g_FrameContext[bufferIdx];
            if (frame.CommandAllocator && g_pCommandList && g_pBackBuffers[bufferIdx] && g_pCommandQueue) {
                // Synchronize with GPU before resetting this buffer's command allocator
                if (g_pFence && frame.FenceValue != 0 && g_pFence->GetCompletedValue() < frame.FenceValue) {
                    g_pFence->SetEventOnCompletion(frame.FenceValue, g_hFenceEvent);
                    WaitForSingleObject(g_hFenceEvent, 2000); // Safe bounded wait
                }

                frame.CommandAllocator->Reset();
                g_pCommandList->Reset(frame.CommandAllocator, nullptr);

                ImGui_ImplDX12_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();

                // Render visible mouse cursor when menu is open
                ImGui::GetIO().MouseDrawCursor = UI::s_MenuOpen;

                UI::Render();

                ImGui::Render();

                D3D12_RESOURCE_BARRIER barrier = {};
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barrier.Transition.pResource = g_pBackBuffers[bufferIdx];
                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
                g_pCommandList->ResourceBarrier(1, &barrier);

                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
                rtvHandle.ptr += bufferIdx * g_RtvDescriptorSize;
                g_pCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

                ID3D12DescriptorHeap* heaps[] = { g_pSrvDescHeap };
                g_pCommandList->SetDescriptorHeaps(1, heaps);

                ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pCommandList);

                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
                g_pCommandList->ResourceBarrier(1, &barrier);

                g_pCommandList->Close();

                ID3D12CommandList* cmdLists[] = { g_pCommandList };
                g_pCommandQueue->ExecuteCommandLists(1, cmdLists);

                if (g_pFence) {
                    g_FenceValue++;
                    g_pCommandQueue->Signal(g_pFence, g_FenceValue);
                    frame.FenceValue = g_FenceValue;
                }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            LOG_ERROR("Caught exception 0x%08X during ImGui Present render! Frame skipped safely.", GetExceptionCode());
        }
    }

    return oPresent(pSwapChain, SyncInterval, Flags);
}

bool D3D12Hook::Init() {
    if (s_Hooked) return true;

    LOG_INFO("Discovering DirectX 12 SwapChain & CommandQueue VTable via dummy device...");

    WNDCLASSA wc = {};
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = "EnhancedImGui_Dummy";
    RegisterClassA(&wc);

    HWND dummyHwnd = CreateWindowA(wc.lpszClassName, "", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr, wc.hInstance, nullptr);
    if (!dummyHwnd) {
        LOG_ERROR("Failed to create dummy window.");
        return false;
    }

    HMODULE hD3D12 = LoadLibraryA("d3d12.dll");
    HMODULE hDXGI = LoadLibraryA("dxgi.dll");
    if (!hD3D12 || !hDXGI) {
        LOG_ERROR("Failed to load d3d12.dll or dxgi.dll.");
        DestroyWindow(dummyHwnd);
        UnregisterClassA(wc.lpszClassName, wc.hInstance);
        return false;
    }

    typedef HRESULT (WINAPI *pfnD3D12CreateDevice)(IUnknown*, D3D_FEATURE_LEVEL, REFIID, void**);
    typedef HRESULT (WINAPI *pfnCreateDXGIFactory)(REFIID, void**);

    auto d3d12CreateDevice = (pfnD3D12CreateDevice)GetProcAddress(hD3D12, "D3D12CreateDevice");
    auto createDXGIFactory = (pfnCreateDXGIFactory)GetProcAddress(hDXGI, "CreateDXGIFactory");

    if (!d3d12CreateDevice || !createDXGIFactory) {
        LOG_ERROR("Failed to get D3D12CreateDevice or CreateDXGIFactory exports.");
        DestroyWindow(dummyHwnd);
        UnregisterClassA(wc.lpszClassName, wc.hInstance);
        return false;
    }

    ID3D12Device* dummyDevice = nullptr;
    if (FAILED(d3d12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dummyDevice)))) {
        LOG_ERROR("Failed to create dummy D3D12 device.");
        DestroyWindow(dummyHwnd);
        UnregisterClassA(wc.lpszClassName, wc.hInstance);
        return false;
    }

    D3D12_COMMAND_QUEUE_DESC qDesc = {};
    qDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ID3D12CommandQueue* dummyQueue = nullptr;
    if (FAILED(dummyDevice->CreateCommandQueue(&qDesc, IID_PPV_ARGS(&dummyQueue)))) {
        LOG_ERROR("Failed to create dummy CommandQueue.");
        dummyDevice->Release();
        DestroyWindow(dummyHwnd);
        UnregisterClassA(wc.lpszClassName, wc.hInstance);
        return false;
    }

    IDXGIFactory* dummyFactory = nullptr;
    if (FAILED(createDXGIFactory(IID_PPV_ARGS(&dummyFactory)))) {
        LOG_ERROR("Failed to create dummy DXGIFactory.");
        dummyQueue->Release();
        dummyDevice->Release();
        DestroyWindow(dummyHwnd);
        UnregisterClassA(wc.lpszClassName, wc.hInstance);
        return false;
    }

    DXGI_SWAP_CHAIN_DESC scDesc = {};
    scDesc.BufferCount = 2;
    scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.BufferDesc.Width = 100;
    scDesc.BufferDesc.Height = 100;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.OutputWindow = dummyHwnd;
    scDesc.SampleDesc.Count = 1;
    scDesc.Windowed = TRUE;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    IDXGISwapChain* dummySwapChain = nullptr;
    if (FAILED(dummyFactory->CreateSwapChain(dummyQueue, &scDesc, &dummySwapChain))) {
        LOG_ERROR("Failed to create dummy SwapChain.");
        dummyFactory->Release();
        dummyQueue->Release();
        dummyDevice->Release();
        DestroyWindow(dummyHwnd);
        UnregisterClassA(wc.lpszClassName, wc.hInstance);
        return false;
    }

    void** pVMTQueue = *(void***)dummyQueue;
    void** pVMTSwapChain = *(void***)dummySwapChain;

    void* pExecuteCommandListsTarget = pVMTQueue[10];
    void* pPresentTarget = pVMTSwapChain[8];
    void* pResizeBuffersTarget = pVMTSwapChain[13];

    LOG_INFO("VTable Targets -> ExecuteCommandLists: %p | Present: %p | ResizeBuffers: %p",
        pExecuteCommandListsTarget, pPresentTarget, pResizeBuffersTarget);

    dummySwapChain->Release();
    dummyFactory->Release();
    dummyQueue->Release();
    dummyDevice->Release();
    DestroyWindow(dummyHwnd);
    UnregisterClassA(wc.lpszClassName, wc.hInstance);

    MH_STATUS mhInit = MH_Initialize();
    if (mhInit != MH_OK && mhInit != MH_ERROR_ALREADY_INITIALIZED) {
        LOG_ERROR("MinHook initialization failed: %d", (int)mhInit);
        return false;
    }

    auto hookMethod = [](void* target, void* detour, void** original) -> bool {
        MH_STATUS s = MH_CreateHook(target, detour, original);
        if (s == MH_OK || s == MH_ERROR_ALREADY_CREATED) {
            return MH_EnableHook(target) == MH_OK;
        }
        LOG_ERROR("MH_CreateHook failed with status %d", (int)s);
        return false;
    };

    if (!hookMethod(pExecuteCommandListsTarget, (void*)&hk_ExecuteCommandLists, (void**)&oExecuteCommandLists) ||
        !hookMethod(pPresentTarget, (void*)&hk_Present, (void**)&oPresent) ||
        !hookMethod(pResizeBuffersTarget, (void*)&hk_ResizeBuffers, (void**)&oResizeBuffers)) {
        LOG_ERROR("Failed to establish MinHook detours on DirectX 12 methods.");
        return false;
    }

    s_Hooked = true;
    LOG_INFO("DirectX 12 hooks successfully established and active.");
    return true;
}

void D3D12Hook::Shutdown() {
    if (s_Hooked) {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
        s_Hooked = false;
    }

    if (s_Hwnd && oWndProc) {
        if (s_WndProcIsUnicode) {
            SetWindowLongPtrW(s_Hwnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);
        } else {
            SetWindowLongPtrA(s_Hwnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);
        }
        oWndProc = nullptr;
    }

    if (g_bImGuiInitialized) {
        WaitForLastSubmittedFrame();
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        CleanupRenderTarget();

        if (g_pFence) {
            g_pFence->Release();
            g_pFence = nullptr;
        }
        if (g_hFenceEvent) {
            CloseHandle(g_hFenceEvent);
            g_hFenceEvent = nullptr;
        }
        for (UINT i = 0; i < 8; ++i) {
            if (g_FrameContext[i].CommandAllocator) {
                g_FrameContext[i].CommandAllocator->Release();
                g_FrameContext[i].CommandAllocator = nullptr;
            }
        }
        if (g_pCommandList) {
            g_pCommandList->Release();
            g_pCommandList = nullptr;
        }
        if (g_pRtvDescHeap) {
            g_pRtvDescHeap->Release();
            g_pRtvDescHeap = nullptr;
        }
        if (g_pSrvDescHeap) {
            g_pSrvDescHeap->Release();
            g_pSrvDescHeap = nullptr;
        }

        g_bImGuiInitialized = false;
    }
}
