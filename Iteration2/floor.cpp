#include "message.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>

extern std::mutex mtx;
extern std::condition_variable cv;
extern std::queue<ElevatorMessage> schedulerQueue;
extern bool systemActive;
extern std::vector<bool> elevatorBusy; // To track elevator state

void floorFunction() {
    std::ifstream inputFile("input.txt");
    if (!inputFile) {
        std::cerr << "Error: Unable to open input.txt. Exiting floorFunction." << std::endl;
        return;
    }

    int requestCount = 0;
    int floor, destination;

    while (systemActive && inputFile >> floor >> destination) { 
        if (floor == destination) continue; // Ignore invalid requests

        {
            std::unique_lock<std::mutex> lk(mtx);
            
            // Wait until elevator is idle before sending a new request
            cv.wait(lk, [] { return !elevatorBusy[0]; });

            ElevatorMessage msg = {requestCount, floor, destination, (destination > floor), 0};
            schedulerQueue.push(msg);
            std::cout << "Floor request: From " << msg.floorNumber 
                      << " to " << msg.destination << std::endl;
        }

        cv.notify_all(); 
        requestCount++;
    }

    inputFile.close();
}
