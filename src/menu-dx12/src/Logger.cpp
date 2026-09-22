#include "Logger.h"
#include <cstdio>
#include <cstdarg>
#include <ctime>

std::ofstream Logger::s_LogFile;
std::mutex Logger::s_Mutex;
bool Logger::s_Initialized = false;
bool Logger::s_ConsoleActive = false;

void Logger::OpenConsole() {
    std::lock_guard<std::mutex> lock(s_Mutex);
    if (s_ConsoleActive) return;

    if (AllocConsole()) {
        FILE* fp = nullptr;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        SetConsoleTitleA("GTA V Enhanced - Mod Menu Debug Console");
        s_ConsoleActive = true;

        printf("\n");
        printf("===============================================================\n");
        printf(" GTA V Enhanced - Live Mod Menu & Hook Diagnostics Console     \n");
        printf("===============================================================\n\n");
    }
}

void Logger::Init(const std::string& logPath) {
    std::lock_guard<std::mutex> lock(s_Mutex);
    if (s_Initialized) return;
    s_LogFile.open(logPath, std::ios::out | std::ios::app);
    s_Initialized = s_LogFile.is_open();
    if (s_Initialized) {
        char timeBuf[64];
        time_t now = time(nullptr);
        struct tm* tmInfo = localtime(&now);
        strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tmInfo);
        s_LogFile << "\n======================================================\n";
        s_LogFile << "[" << timeBuf << "] EnhancedImGuiMenu initialized for GTA V Enhanced Edition\n";
        s_LogFile << "======================================================\n";
        s_LogFile.flush();
    }
}

void Logger::Close() {
    std::lock_guard<std::mutex> lock(s_Mutex);
    if (s_Initialized && s_LogFile.is_open()) {
        s_LogFile << "[SHUTDOWN] EnhancedImGuiMenu closing cleanly.\n";
        s_LogFile.flush();
        s_LogFile.close();
        s_Initialized = false;
    }
    if (s_ConsoleActive) {
        FreeConsole();
        s_ConsoleActive = false;
    }
}

void Logger::Log(const char* level, const char* format, ...) {
    std::lock_guard<std::mutex> lock(s_Mutex);
    if (!s_Initialized || !s_LogFile.is_open()) return;

    char timeBuf[32];
    time_t now = time(nullptr);
    struct tm* tmInfo = localtime(&now);
    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", tmInfo);

    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    s_LogFile << "[" << timeBuf << "] [" << level << "] " << buffer << "\n";
    s_LogFile.flush();

    if (s_ConsoleActive) {
        WORD color = 7;
        if (strcmp(level, "ERROR") == 0) color = 12; // Red
        else if (strcmp(level, "WARN") == 0) color = 14; // Yellow
        else if (strcmp(level, "INFO") == 0) color = 11; // Cyan

        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, 8);
        printf("[%s] ", timeBuf);
        SetConsoleTextAttribute(hConsole, color);
        printf("[%s] ", level);
        SetConsoleTextAttribute(hConsole, 15);
        printf("%s\n", buffer);
        SetConsoleTextAttribute(hConsole, 7);
    }
}
