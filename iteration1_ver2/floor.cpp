#include "message.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

extern std::mutex mtx;
extern std::condition_variable cv;
extern std::queue<ElevatorMessage> schedulerQueue;
extern bool systemActive;

void floorFunction() {
    int requestCount = 0;
    while (systemActive && requestCount < 10) { // Generate 10 requests
        int floor = rand() % 10 + 1;  // Random floor 1-10
        int destination = rand() % 10 + 1;
        while (destination == floor) {
            destination = rand() % 10 + 1; // Ensure different destination
        }

        ElevatorMessage msg = {requestCount, floor, destination, (destination > floor), 0};

        {
            std::lock_guard<std::mutex> lk(mtx);
            schedulerQueue.push(msg);
            std::cout << " Floor request: From " << msg.floorNumber << " to " << msg.destination << std::endl;
        }
        cv.notify_all(); // Notify other subsystems
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Simulate delay
        requestCount++;
    }
}
