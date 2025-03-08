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
        cv.wait(lk, [] { return !elevatorQueue.empty(); });

        ElevatorMessage request = elevatorQueue.front();
        elevatorQueue.pop();
        elevatorBusy[elevatorId] = true; // Mark elevator as busy
        lk.unlock();

        // Move to the pickup floor
        int currentFloor = elevatorPositions[elevatorId];
        int pickupFloor = request.floorNumber;
        int destinationFloor = request.destination;

        if (currentFloor < pickupFloor) {
            state = ElevatorState::MOVING;
            std::cout << "Elevator " << elevatorId << " moving up to pickup Floor " << pickupFloor << std::endl;
        } else if (currentFloor > pickupFloor) {
            state = ElevatorState::MOVING;
            std::cout << "Elevator " << elevatorId << " moving down to pickup Floor " << pickupFloor << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Simulate travel time

        state = ElevatorState::STOPPING;
        std::cout << "Elevator " << elevatorId << " stopping at pickup Floor " << pickupFloor << std::endl;

        state = ElevatorState::DOOR_OPEN;
        std::cout << "Elevator " << elevatorId << " doors opening at pickup Floor " << pickupFloor << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Simulate door open time

        state = ElevatorState::DOOR_CLOSED;
        std::cout << "Elevator " << elevatorId << " doors closing at pickup Floor " << pickupFloor << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Simulate door close time

        // Move to the destination floor
        currentFloor = pickupFloor;
        if (currentFloor < destinationFloor) {
            state = ElevatorState::MOVING;
            std::cout << "Elevator " << elevatorId << " moving up to destination Floor " << destinationFloor << std::endl;
        } else if (currentFloor > destinationFloor) {
            state = ElevatorState::MOVING;
            std::cout << "Elevator " << elevatorId << " moving down to destination Floor " << destinationFloor << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Simulate travel time

        state = ElevatorState::STOPPING;
        std::cout << "Elevator " << elevatorId << " stopping at destination Floor " << destinationFloor << std::endl;

        state = ElevatorState::DOOR_OPEN;
        std::cout << "Elevator " << elevatorId << " doors opening at destination Floor " << destinationFloor << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Simulate door open time

        state = ElevatorState::DOOR_CLOSED;
        std::cout << "Elevator " << elevatorId << " doors closing at destination Floor " << destinationFloor << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Simulate door close time

        {
            std::lock_guard<std::mutex> lock(mtx);
            elevatorPositions[elevatorId] = destinationFloor;
            elevatorBusy[elevatorId] = false; // Mark elevator as free
            std::cout << "Elevator " << elevatorId << " is now idle." << std::endl;
        }

        // Notify scheduler about the elevator's current position and state
        cv.notify_all(); // Notify scheduler and floor to send the next request
    }
}
