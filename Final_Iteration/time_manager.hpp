#ifndef TIME_MANAGER_HPP
#define TIME_MANAGER_HPP

#include <atomic>

// Global variables for simulation time and movement counting.
extern std::atomic<int> currentTime;
extern std::atomic<int> totalMovements;

void updateTime(int newTime);

#endif // TIME_MANAGER_HPP
