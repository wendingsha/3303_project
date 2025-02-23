#include "message.hpp"
#include "scheduler.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <algorithm>

extern std::mutex mtx;
extern std::condition_variable cv;
extern std::queue<ElevatorMessage> schedulerQueue;
extern std::queue<ElevatorMessage> elevatorQueue;
extern std::vector<int> elevatorPositions;
extern bool systemActive;
std::vector<bool> elevatorBusy(1, false); // Single elevator

void schedulerFunction() {
    while (systemActive) {
        std::unique_lock<std::mutex> lk(mtx);

        if (schedulerQueue.empty()) {
            cv.wait(lk, [] { return !schedulerQueue.empty(); });
        }

        // Wait for elevator to be free before assigning a new request
        cv.wait(lk, [] { return !elevatorBusy[0]; });

        ElevatorMessage request = schedulerQueue.front();
        schedulerQueue.pop();
        lk.unlock();

        int bestElevator = 0; // Only one elevator

        request.assignedElevator = bestElevator;
        elevatorBusy[bestElevator] = true; // Mark elevator as busy

        {
            std::lock_guard<std::mutex> lock(mtx);
            elevatorQueue.push(request);
            std::cout << "Scheduler assigned Floor " << request.source
                      << " to Elevator " << bestElevator << std::endl;
        }

        cv.notify_all();
    }
}