#ifndef TIME_MANAGER_HPP
#define TIME_MANAGER_HPP

#include <atomic>

extern std::atomic<int> currentTime;
void updateTime(int newTime);

#endif // TIME_MANAGER_HPP
