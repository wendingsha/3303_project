#include "message.hpp"
#include "floor.hpp"
#include "scheduler.hpp"
#include "elevator.hpp"
#include <thread>
#include <vector>
#include <iostream>

// Define systemActive globally
bool systemActive = true;

int main() {
    std::thread floorThread(floorFunction);
    std::thread schedulerThread(schedulerFunction);

    // Start multiple elevators
    std::vector<std::thread> elevatorThreads;
    for (int i = 0; i < MAX_ELEVATORS; i++) {
        elevatorThreads.emplace_back(elevatorFunction, i);
    }

    std::this_thread::sleep_for(std::chrono::seconds(60));
    systemActive = false; // Stop simulation

    floorThread.join();
    schedulerThread.join();
    for (auto &thread : elevatorThreads) {
        thread.join();
    }

    return 0;
}
