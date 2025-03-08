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

enum class SchedulerState {
    IDLE,
    WAITING_FOR_REQUEST,
    WAITING_FOR_ELEVATOR,
    ASSIGNING_REQUEST
};

void schedulerFunction() {
    SchedulerState state = SchedulerState::IDLE;

    while (systemActive) {
        std::unique_lock<std::mutex> lk(mtx);

        switch (state) {
            case SchedulerState::IDLE:
                state = SchedulerState::WAITING_FOR_REQUEST;
                break;

            case SchedulerState::WAITING_FOR_REQUEST:
                cv.wait(lk, [] { return !schedulerQueue.empty(); });
                state = SchedulerState::WAITING_FOR_ELEVATOR;
                break;

            case SchedulerState::WAITING_FOR_ELEVATOR:
                cv.wait(lk, [] { return !elevatorBusy[0]; });
                state = SchedulerState::ASSIGNING_REQUEST;
                break;

            case SchedulerState::ASSIGNING_REQUEST: {
                ElevatorMessage request = schedulerQueue.front();
                schedulerQueue.pop();
                lk.unlock();

                int bestElevator = 0; // Only one elevator
                request.assignedElevator = bestElevator;
                elevatorBusy[bestElevator] = true;

                {
                    std::lock_guard<std::mutex> lock(mtx);
                    elevatorQueue.push(request);
                    std::cout << "Scheduler assigned Floor " << request.floorNumber
                              << " to Elevator " << bestElevator << std::endl;
                }

                cv.notify_all();
                state = SchedulerState::IDLE;
                break;
            }
        }
    }
}
