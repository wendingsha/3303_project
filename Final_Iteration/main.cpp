#include "message.hpp"
#include "floor.hpp"
#include "scheduler.hpp"
#include "elevator.hpp"
#include "time_manager.hpp"
#include <thread>
#include <vector>
#include <iostream>

bool systemActive = true;

int main() {
    std::thread floorThread(floorFunction);
    std::thread schedulerThread(schedulerFunction);
    
    // Launch dashboard thread from the scheduler to show a consolidated status.
    std::thread dashboardThread(displayDashboard);

    // Launch elevator threads (as defined by MAX_ELEVATORS in scheduler.cpp).
    std::vector<std::thread> elevatorThreads;
    for (int i = 0; i < 4; i++) { 
        elevatorThreads.emplace_back(elevatorFunction, i);
    }

    std::cout << "Press Enter to stop simulation and output performance metrics..." << std::endl;
    std::cin.get();  // Wait for Enter key.
    systemActive = false; // Signal threads to stop.

    floorThread.join();
    schedulerThread.join();
    dashboardThread.join();
    for (auto &thread : elevatorThreads) {
        thread.join();
    }

    // Output  metrics.
    std::cout << "\n=== Performance Metrics ===" << std::endl;
    std::cout << "Total simulation time: " << currentTime.load() << " seconds" << std::endl;
    std::cout << "Total floor movements: " << totalMovements.load() << std::endl;
    std::cout << "===========================" << std::endl;

    return 0;
}
