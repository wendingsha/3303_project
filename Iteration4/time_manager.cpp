#include "time_manager.hpp"
#include <mutex>

std::atomic<int> currentTime(0);
static std::mutex timeMutex;

void updateTime(int newTime) {
    std::lock_guard<std::mutex> lock(timeMutex);
    if (newTime > currentTime.load()) {
        currentTime.store(newTime);
    }
}
