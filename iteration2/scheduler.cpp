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

        ElevatorMessage request = schedulerQueue.front();
        schedulerQueue.pop();
        lk.unlock();

        int bestElevator = 0; // Only one elevator, always assign it
        
        request.assignedElevator = bestElevator;
        elevatorBusy[bestElevator] = true;

        {
            std::lock_guard<std::mutex> lk(mtx);
            elevatorQueue.push(request);
            std::cout << "Scheduler assigned Floor " << request.floorNumber
                      << " to Elevator " << bestElevator << std::endl;
        }

        cv.notify_all();
    }
}
