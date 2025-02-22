#include "message.hpp"
#include "elevator.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <chrono>

extern std::mutex mtx;
extern std::condition_variable cv;
extern std::queue<ElevatorMessage> elevatorQueue;
extern std::vector<int> elevatorPositions;
extern bool systemActive;
extern std::vector<bool> elevatorBusy;

void elevatorFunction(int elevatorId) {
    ElevatorState state = ElevatorState::IDLE;

    while (systemActive) {
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, [elevatorId] { return !elevatorQueue.empty(); });

        ElevatorMessage request;
        bool found = false;
        std::queue<ElevatorMessage> tempQueue;

        while (!elevatorQueue.empty()) {
            ElevatorMessage msg = elevatorQueue.front();
            elevatorQueue.pop();

            if (msg.assignedElevator == elevatorId && !found) {
                request = msg;
                found = true;
            } else {
                tempQueue.push(msg);
            }
        }
        elevatorQueue = std::move(tempQueue);

        if (!found) {
            lk.unlock();
            continue;
        }
        lk.unlock();

        state = ElevatorState::MOVING;
        std::cout << "Elevator " << elevatorId << " moving to Floor " << request.floorNumber << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        state = ElevatorState::STOPPING;
        std::cout << "Elevator " << elevatorId << " stopping at Floor " << request.floorNumber << std::endl;

        state = ElevatorState::DOOR_OPEN;
        std::cout << "Elevator " << elevatorId << " doors opening." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));

        state = ElevatorState::DOOR_CLOSED;
        std::cout << "Elevator " << elevatorId << " doors closing." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));

        state = ElevatorState::MOVING;
        std::cout << "Elevator " << elevatorId << " moving to destination Floor " << request.destination << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        state = ElevatorState::STOPPING;
        std::cout << "Elevator " << elevatorId << " stopping at Floor " << request.destination << std::endl;

        state = ElevatorState::DOOR_OPEN;
        std::cout << "Elevator " << elevatorId << " doors opening." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));

        state = ElevatorState::DOOR_CLOSED;
        std::cout << "Elevator " << elevatorId << " doors closing." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));

        {
            std::lock_guard<std::mutex> lock(mtx);
            elevatorPositions[elevatorId] = request.destination;
            elevatorBusy[elevatorId] = false;
        }

        state = ElevatorState::IDLE;
        std::cout << "Elevator " << elevatorId << " is now idle." << std::endl;

        cv.notify_all();
    }
}

