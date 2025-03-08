#include "time_manager.hpp"
#include <unistd.h>

int currentTime = 0;  

void updateTime(int newTime) {
    if (newTime > currentTime) {
        sleep(newTime - currentTime);
        currentTime = newTime;
    }
}