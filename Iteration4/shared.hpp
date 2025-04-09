// shared.hpp
#ifndef SHARED_HPP
#define SHARED_HPP

#include <vector>
#include <string>
#include <mutex>

struct ElevatorStatus {
    int id = 0;
    int currentFloor = 1;
    int destination = -1;
    std::string status = "Idle";
    bool fault = false;
};

extern std::vector<ElevatorStatus> elevatorStatuses;
extern std::mutex statusMutex;

#endif // SHARED_HPP
