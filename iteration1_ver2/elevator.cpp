#include "message.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>

extern std::mutex mtx;
extern std::condition_variable cv;
extern std::queue<ElevatorMessage> elevatorQueue;
extern std::vector<int> elevatorPositions;
extern bool systemActive;
extern std::vector<bool> elevatorBusy; 

void elevatorFunction(int elevatorId) {
    while (systemActive) {
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, [elevatorId] {
            return !elevatorQueue.empty();  // Wait for new requests
        });

        ElevatorMessage request;
        bool found = false;

        // Assign request to the best fit elevator
        std::queue<ElevatorMessage> tempQueue;
        while (!elevatorQueue.empty()) {
            ElevatorMessage msg = elevatorQueue.front();
            elevatorQueue.pop();

            if (msg.assignedElevator == elevatorId && !found) {
                request = msg; // Assign request to this elevator
                found = true;
            } else {
                tempQueue.push(msg); // Keep other requests
            }
        }

        // Restore remaining requests back to the elevatorQueue
        elevatorQueue = std::move(tempQueue);

        if (!found) {
            lk.unlock();
            continue;  // No request found for this elevator
        }

        lk.unlock();

        std::cout << "Elevator " << elevatorId << " moving to Floor " 
                  << request.floorNumber << " to pick up passenger." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        std::cout << "Elevator " << elevatorId << " taking passenger to Floor " 
                  << request.destination << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        std::cout << "Elevator " << elevatorId << " arrived at Floor " << request.destination << std::endl;

        // Update position and mark elevator as available again
        {
            std::lock_guard<std::mutex> lock(mtx);
            elevatorPositions[elevatorId] = request.destination;
            elevatorBusy[elevatorId] = false; // Mark elevator as free
        }

        cv.notify_all(); // Notify other subsystems 
    }
}


