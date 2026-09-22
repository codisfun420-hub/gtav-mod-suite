#pragma once
#include <windows.h>
#include <string>
#include <mutex>
#include <fstream>

class Logger {
public:
    static void Init(const std::string& logPath);
    static void Close();
    static void Log(const char* level, const char* format, ...);
    static void OpenConsole();

private:
    static std::ofstream s_LogFile;
    static std::mutex s_Mutex;
    static bool s_Initialized;
    static bool s_ConsoleActive;
};

#define LOG_INFO(fmt, ...)  Logger::Log("INFO", fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  Logger::Log("WARN", fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger::Log("ERROR", fmt, ##__VA_ARGS__)
