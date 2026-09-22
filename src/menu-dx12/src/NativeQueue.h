#pragma once
#include <vector>
#include <functional>
#include <mutex>

class NativeQueue {
public:
    static void Push(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(s_Mutex);
        s_Queue.push_back(task);
    }

    static void ProcessAll() {
        std::vector<std::function<void()>> localQueue;
        {
            std::lock_guard<std::mutex> lock(s_Mutex);
            if (s_Queue.empty()) return;
            localQueue.swap(s_Queue);
        }

        for (auto& task : localQueue) {
            if (task) {
                try {
                    task();
                } catch (...) {
                    // Suppress any native exceptions gracefully to protect the game thread
                }
            }
        }
    }

private:
    static inline std::vector<std::function<void()>> s_Queue;
    static inline std::mutex s_Mutex;
};
