#include "message.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>

extern std::mutex mtx;
extern std::condition_variable cv;
extern std::queue<ElevatorMessage> schedulerQueue;
extern std::queue<ElevatorMessage> elevatorQueue;
extern std::vector<int> elevatorPositions;
extern bool systemActive;

std::vector<bool> elevatorBusy(2, false); // Track if elevators are in use

void schedulerFunction() {
    while (systemActive) {
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, [] { return !schedulerQueue.empty(); }); // Wait for requests

        ElevatorMessage request = schedulerQueue.front();
        schedulerQueue.pop();
        lk.unlock();

        int bestElevator = -1;
        int minDist = 1000; // Large number to start with

        // First, check for an idle elevator
        for (int i = 0; i < elevatorPositions.size(); i++) {
            if (!elevatorBusy[i]) { // If an elevator is free, assign the request immediately
                bestElevator = i;
                minDist = abs(elevatorPositions[i] - request.floorNumber);
                break;  // Stop searching because we found a free elevator
            }
        }

        // If no elevator is free, assign to the closest busy one
        if (bestElevator == -1) {
            for (int i = 0; i < elevatorPositions.size(); i++) {
                int dist = abs(elevatorPositions[i] - request.floorNumber);
                if (dist < minDist) {
                    bestElevator = i;
                    minDist = dist;
                }
            }
        }

        request.assignedElevator = bestElevator;
        elevatorBusy[bestElevator] = true; // Mark elevator as busy

        {
            std::lock_guard<std::mutex> lk(mtx);
            elevatorQueue.push(request);
            std::cout << "Scheduler assigned request from Floor " << request.floorNumber
                      << " to Elevator " << bestElevator << std::endl;
        }

        cv.notify_all(); // Notify elevators
    }
}


