#include "CrashHandler.h"
#include "Logger.h"
#include <cstdio>
#include <cstring>
#include <dbghelp.h>

static PVOID g_pVehHandle = nullptr;

static LONG WINAPI VectoredCrashHandler(EXCEPTION_POINTERS* pExInfo) {
    if (!pExInfo || !pExInfo->ExceptionRecord) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    DWORD code = pExInfo->ExceptionRecord->ExceptionCode;

    // Only intercept severe fatal crash exceptions
    if (code == EXCEPTION_ACCESS_VIOLATION ||
        code == EXCEPTION_ILLEGAL_INSTRUCTION ||
        code == EXCEPTION_STACK_OVERFLOW ||
        code == EXCEPTION_INT_DIVIDE_BY_ZERO ||
        code == 0xC0000025) // STATUS_NONCONTINUABLE_EXCEPTION
    {
        DWORD_PTR faultAddr = (DWORD_PTR)pExInfo->ExceptionRecord->ExceptionAddress;
        HMODULE hMod = nullptr;
        char modPath[MAX_PATH] = "UnknownModule";
        DWORD_PTR rva = 0;

        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)faultAddr, &hMod) && hMod) {
            GetModuleFileNameA(hMod, modPath, sizeof(modPath));
            char* slash = strrchr(modPath, '\\');
            if (slash) {
                memmove(modPath, slash + 1, strlen(slash + 1) + 1);
            }
            rva = faultAddr - (DWORD_PTR)hMod;
        }

        // Auto-heal known GTA5_Enhanced.exe uninitialized Steam interface overlay crashes
        if (code == EXCEPTION_ACCESS_VIOLATION && _stricmp(modPath, "GTA5_Enhanced.exe") == 0) {
            // Function 1: 0x1E963A0 - 0x1E96437 (Steam user/utils query)
            if (rva >= 0x1E963A0 && rva <= 0x1E96437) {
                LOG_WARN("================================================================================");
                LOG_WARN("[AUTO-HEALED] Intercepted GTA5_Enhanced.exe Steam overlay crash #1 at +0x%IX!", rva);
                LOG_WARN("Safely skipping uninitialized Steam interface call and resuming execution at +0x1E96423...");
                LOG_WARN("================================================================================");
                if (pExInfo->ContextRecord) {
                    pExInfo->ContextRecord->Rip = (DWORD_PTR)hMod + 0x1E96423;
                    pExInfo->ContextRecord->Rax = 0; // Return FALSE / safe failure
                }
                return EXCEPTION_CONTINUE_EXECUTION;
            }

            // Function 2: 0x1E96440 - 0x1E9654E (Steam friends/overlay callback)
            if (rva >= 0x1E96440 && rva <= 0x1E9654E) {
                LOG_WARN("================================================================================");
                LOG_WARN("[AUTO-HEALED] Intercepted GTA5_Enhanced.exe Steam overlay crash #2 at +0x%IX!", rva);
                LOG_WARN("Safely unwinding 0x48 stack frame, releasing lock, and resuming execution at +0x1E9646E...");
                LOG_WARN("================================================================================");
                if (pExInfo->ContextRecord) {
                    pExInfo->ContextRecord->Rip = (DWORD_PTR)hMod + 0x1E9646E;
                    pExInfo->ContextRecord->Rax = 0; // Return FALSE / safe failure
                }
                return EXCEPTION_CONTINUE_EXECUTION;
            }

            // Function 3: 0x1E7A2A0 - 0x1E7A349 (Steam overlay / game stats sync query at +0x1E7A330)
            if (rva >= 0x1E7A2A0 && rva <= 0x1E7A349) {
                LOG_WARN("================================================================================");
                LOG_WARN("[AUTO-HEALED] Intercepted GTA5_Enhanced.exe Steam interface crash #3 at +0x%IX!", rva);
                LOG_WARN("Safely unwinding stack frame (add rsp, 0x38; pop rbx, rbp, rdi, rsi; ret) at +0x1E7A33F...");
                LOG_WARN("================================================================================");
                if (pExInfo->ContextRecord) {
                    pExInfo->ContextRecord->Rip = (DWORD_PTR)hMod + 0x1E7A33F;
                    pExInfo->ContextRecord->Rax = 1; // Return TRUE / safely handled
                }
                return EXCEPTION_CONTINUE_EXECUTION;
            }

            // Function 4: 0x1E7F940 - 0x1E7FA6B (Steam interface stats/sync callback at +0x1E7F978)
            // Stack layout: push RSI, push RDI, push RBP, push RBX, sub rsp, 0x98
            // Frame size: 4*8 + 0x98 = 0xB8 bytes
            // Return address is at [RSP + 0xB8], saved regs at [RSP + 0x98..0xB0]
            if (rva >= 0x1E7F940 && rva <= 0x1E7FA6B) {
                LOG_WARN("================================================================================");
                LOG_WARN("[AUTO-HEALED] Intercepted GTA5_Enhanced.exe Steam interface crash #4 at +0x%IX!", rva);
                LOG_WARN("Manually unwinding 0x98 frame (push RSI/RDI/RBP/RBX + alloc 0x98) and returning...");
                LOG_WARN("================================================================================");
                if (pExInfo->ContextRecord) {
                    // Restore the saved non-volatile registers from the stack
                    auto* stack = (ULONG_PTR*)pExInfo->ContextRecord->Rsp;
                    pExInfo->ContextRecord->Rbx = stack[0x98 / 8];
                    pExInfo->ContextRecord->Rbp = stack[0xA0 / 8];
                    pExInfo->ContextRecord->Rdi = stack[0xA8 / 8];
                    pExInfo->ContextRecord->Rsi = stack[0xB0 / 8];
                    // Load return address and unwind RSP (equivalent to epilogue + ret)
                    pExInfo->ContextRecord->Rip = stack[0xB8 / 8];
                    pExInfo->ContextRecord->Rsp += 0xC0; // 0xB8 frame + 8 for the ret pop
                    pExInfo->ContextRecord->Rax = 0;     // Return FALSE / safe failure
                }
                return EXCEPTION_CONTINUE_EXECUTION;
            }

            // Function 5: +0x1E97340 (Steam interface getter / mov rax, [rdx+0x48]; ret with RDX=0)
            if (rva >= 0x1E97330 && rva <= 0x1E97350) {
                LOG_WARN("================================================================================");
                LOG_WARN("[AUTO-HEALED] Intercepted GTA5_Enhanced.exe Steam interface crash #5 at +0x%IX!", rva);
                LOG_WARN("Safely returning NULL (mov rax, 0; ret)...");
                LOG_WARN("================================================================================");
                if (pExInfo->ContextRecord) {
                    auto* stack = (ULONG_PTR*)pExInfo->ContextRecord->Rsp;
                    pExInfo->ContextRecord->Rip = stack[0]; // Pop return address
                    pExInfo->ContextRecord->Rsp += 8;
                    pExInfo->ContextRecord->Rax = 0;        // Return NULL / 0
                }
                return EXCEPTION_CONTINUE_EXECUTION;
            }

            // Function 6: +0x1197F1D (movzx ecx, byte ptr [rcx + 0x1f] where rcx=0)
            // Jump safely to the NULL check branch at +0x1197FB5
            if (rva >= 0x1197F10 && rva <= 0x1197F25) {
                LOG_WARN("================================================================================");
                LOG_WARN("[AUTO-HEALED] Intercepted GTA5_Enhanced.exe null dereference at +0x%IX!", rva);
                LOG_WARN("Redirecting to safe null branch at +0x1197FB5...");
                LOG_WARN("================================================================================");
                if (pExInfo->ContextRecord) {
                    pExInfo->ContextRecord->Rip = (DWORD_PTR)hMod + 0x1197FB5;
                }
                return EXCEPTION_CONTINUE_EXECUTION;
            }

            // Function 7: +0x1197CE7 (movzx r9d, word ptr [rcx + 0x18] where rcx=0)
            // Jump safely to the fallback branch at +0x1197E28
            if (rva >= 0x1197CE0 && rva <= 0x1197CF0) {
                LOG_WARN("================================================================================");
                LOG_WARN("[AUTO-HEALED] Intercepted GTA5_Enhanced.exe null dereference at +0x%IX!", rva);
                LOG_WARN("Redirecting to safe fallback branch at +0x1197E28...");
                LOG_WARN("================================================================================");
                if (pExInfo->ContextRecord) {
                    pExInfo->ContextRecord->Rip = (DWORD_PTR)hMod + 0x1197E28;
                }
                return EXCEPTION_CONTINUE_EXECUTION;
            }
        }

        // Auto-heal AMD FSR / GPU driver null pointer crash during window transition
        if (code == EXCEPTION_ACCESS_VIOLATION &&
            (_stricmp(modPath, "amd_fidelityfx_dx12.dll") == 0 || _stricmp(modPath, "amdxc64.dll") == 0)) {
            LOG_WARN("================================================================================");
            LOG_WARN("[AUTO-HEALED] Intercepted GPU driver/FSR crash in %s at +0x%IX!", modPath, rva);
            LOG_WARN("Safely returning failure code to caller...");
            LOG_WARN("================================================================================");
            if (pExInfo->ContextRecord) {
                auto* stack = (ULONG_PTR*)pExInfo->ContextRecord->Rsp;
                pExInfo->ContextRecord->Rip = stack[0]; // Pop return address
                pExInfo->ContextRecord->Rsp += 8;
                pExInfo->ContextRecord->Rax = 0;        // Return 0 / FALSE / DXGI_ERROR_INVALID_CALL
            }
            return EXCEPTION_CONTINUE_EXECUTION;
        }

        LOG_ERROR("================================================================================");
        LOG_ERROR("[CRASH DETECTED] Exception Code: 0x%08X in %s (+0x%IX)", code, modPath, rva);
        LOG_ERROR("Faulting Instruction Address: %p", (void*)faultAddr);

        if (code == EXCEPTION_ACCESS_VIOLATION && pExInfo->ExceptionRecord->NumberParameters >= 2) {
            ULONG_PTR opType = pExInfo->ExceptionRecord->ExceptionInformation[0];
            ULONG_PTR targetAddr = pExInfo->ExceptionRecord->ExceptionInformation[1];
            const char* opStr = (opType == 0) ? "READ" : ((opType == 1) ? "WRITE" : "EXECUTE");
            LOG_ERROR("Memory Access Violation: Tried to %s address %p", opStr, (void*)targetAddr);
        }

        if (pExInfo->ContextRecord) {
            LOG_ERROR("CPU Context Registers:");
            LOG_ERROR("  RIP: %p | RSP: %p | RBP: %p",
                (void*)pExInfo->ContextRecord->Rip,
                (void*)pExInfo->ContextRecord->Rsp,
                (void*)pExInfo->ContextRecord->Rbp);
            LOG_ERROR("  RAX: %p | RBX: %p | RCX: %p | RDX: %p",
                (void*)pExInfo->ContextRecord->Rax,
                (void*)pExInfo->ContextRecord->Rbx,
                (void*)pExInfo->ContextRecord->Rcx,
                (void*)pExInfo->ContextRecord->Rdx);
            LOG_ERROR("  RSI: %p | RDI: %p | R8:  %p | R9:  %p",
                (void*)pExInfo->ContextRecord->Rsi,
                (void*)pExInfo->ContextRecord->Rdi,
                (void*)pExInfo->ContextRecord->R8,
                (void*)pExInfo->ContextRecord->R9);
            LOG_ERROR("  R10: %p | R11: %p | R12: %p | R13: %p",
                (void*)pExInfo->ContextRecord->R10,
                (void*)pExInfo->ContextRecord->R11,
                (void*)pExInfo->ContextRecord->R12,
                (void*)pExInfo->ContextRecord->R13);
            LOG_ERROR("  R14: %p | R15: %p | EFLAGS: 0x%08X",
                (void*)pExInfo->ContextRecord->R14,
                (void*)pExInfo->ContextRecord->R15,
                pExInfo->ContextRecord->EFlags);
        }
        LOG_ERROR("================================================================================");

        // Attempt minidump creation
        HMODULE hDbgHelp = LoadLibraryA("dbghelp.dll");
        if (hDbgHelp) {
            typedef BOOL (WINAPI *pfnMiniDumpWriteDump)(
                HANDLE hProcess,
                DWORD ProcessId,
                HANDLE hFile,
                MINIDUMP_TYPE DumpType,
                PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
                PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                PMINIDUMP_CALLBACK_INFORMATION CallbackParam
            );
            auto pMiniDump = (pfnMiniDumpWriteDump)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
            if (pMiniDump) {
                HANDLE hDump = CreateFileA("EnhancedImGuiMenu_crash.dmp", GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (hDump != INVALID_HANDLE_VALUE) {
                    MINIDUMP_EXCEPTION_INFORMATION mdei;
                    mdei.ThreadId = GetCurrentThreadId();
                    mdei.ExceptionPointers = pExInfo;
                    mdei.ClientPointers = FALSE;
                    if (pMiniDump(GetCurrentProcess(), GetCurrentProcessId(), hDump, MiniDumpNormal, &mdei, nullptr, nullptr)) {
                        LOG_INFO("Crash minidump generated: EnhancedImGuiMenu_crash.dmp");
                    }
                    CloseHandle(hDump);
                }
            }
        }
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

void CrashHandler::Init() {
    if (!g_pVehHandle) {
        g_pVehHandle = AddVectoredExceptionHandler(1, VectoredCrashHandler);
        if (g_pVehHandle) {
            LOG_INFO("Global Vectored Exception Handler installed successfully.");
        }
    }

    // Proactively neutralize Steam Overlay in-process hook if loaded
    HMODULE hOverlay = GetModuleHandleA("gameoverlayrenderer64.dll");
    if (hOverlay) {
        auto pIsOverlayEnabled = (const BYTE*)GetProcAddress(hOverlay, "IsOverlayEnabled");
        if (pIsOverlayEnabled && pIsOverlayEnabled[0] == 0x0F && pIsOverlayEnabled[1] == 0xB6 && pIsOverlayEnabled[2] == 0x05) {
            int32_t disp = *(const int32_t*)(pIsOverlayEnabled + 3);
            uint8_t* pFlag = (uint8_t*)(pIsOverlayEnabled + 7 + disp);
            DWORD oldProtect;
            if (VirtualProtect(pFlag, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                *pFlag = 0;
                VirtualProtect(pFlag, 1, oldProtect, &oldProtect);
                LOG_INFO("Steam In-Game Overlay flag safely neutralized in process memory (bOverlayEnabled = 0).");
            }
        }
    }
}

void CrashHandler::Shutdown() {
    if (g_pVehHandle) {
        RemoveVectoredExceptionHandler(g_pVehHandle);
        g_pVehHandle = nullptr;
        LOG_INFO("Global Vectored Exception Handler removed.");
    }
}
