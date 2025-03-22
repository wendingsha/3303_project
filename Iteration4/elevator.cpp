/* elevator.cpp */
#include "message.hpp"
#include "time_manager.hpp"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <cstdlib>  // for abs()

extern std::atomic<int> currentTime;
extern std::mutex printMutex;
extern bool systemActive;

#define ELEVATOR_PORT_BASE 9100
#define SCHEDULER_IP "127.0.0.1"
#define SCHEDULER_PORT 8100
#define FLOOR_TRAVEL_TIME 1  // seconds per floor
#define MIN_FLOOR 1

// Fault codes
#define NO_FAULT 0
#define DOOR_FAULT 1
#define STUCK_FAULT 2

void elevatorFunction(int elevatorId) {
    ElevatorState elevatorState = IDLE;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket\n";
        return;
    }

    struct sockaddr_in elevatorAddr, schedulerAddr;
    memset(&elevatorAddr, 0, sizeof(elevatorAddr));
    memset(&schedulerAddr, 0, sizeof(schedulerAddr));

    elevatorAddr.sin_family = AF_INET;
    elevatorAddr.sin_addr.s_addr = INADDR_ANY;
    elevatorAddr.sin_port = htons(ELEVATOR_PORT_BASE + elevatorId);

    if (bind(sockfd, (struct sockaddr*)&elevatorAddr, sizeof(elevatorAddr)) < 0) {
        std::cerr << "Bind failed\n";
        return;
    }

    schedulerAddr.sin_family = AF_INET;
    schedulerAddr.sin_port = htons(SCHEDULER_PORT);
    inet_pton(AF_INET, SCHEDULER_IP, &schedulerAddr.sin_addr);

    {
        std::lock_guard<std::mutex> lock(printMutex);
        std::cout << "[ELEVATOR " << elevatorId << "] Listening for assignments...\n";
    }

    ElevatorMessage request;
    socklen_t addrLen = sizeof(schedulerAddr);
    int currentFloor = MIN_FLOOR;
    bool isIdle = true;

    while (systemActive) {
        if (isIdle) {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[ELEVATOR " << elevatorId << "] Waiting for next assignment...\n";
        }

        memset(&request, 0, sizeof(request));
        recvfrom(sockfd, &request, sizeof(request), 0, (struct sockaddr*)&schedulerAddr, &addrLen);
        updateTime(request.timestamp);

        // Check for fault injection
        if (request.faultCode == DOOR_FAULT) {
            {
                std::lock_guard<std::mutex> lock(printMutex);
                std::cout << "[ELEVATOR " << elevatorId << "] Simulating DOOR FAULT at floor " << request.floorNumber << "\n";
            }
            elevatorState = DOOR_OPEN;
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            request.status = -1;
            request.msgType = 2; // fault
            request.timestamp = currentTime.load();
            sendto(sockfd, &request, sizeof(request), 0, (struct sockaddr*)&schedulerAddr, addrLen);
            continue;
        } else if (request.faultCode == STUCK_FAULT) {
            {
                std::lock_guard<std::mutex> lock(printMutex);
                std::cout << "[ELEVATOR " << elevatorId << "] Simulating STUCK FAULT while moving...\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10000));
            request.status = -2;
            request.msgType = 2;
            request.timestamp = currentTime.load();
            sendto(sockfd, &request, sizeof(request), 0, (struct sockaddr*)&schedulerAddr, addrLen);
            continue;
        }

        // Ignore same-floor requests
        if (request.floorNumber == request.destination) {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[ELEVATOR " << elevatorId << "] Ignoring same-floor request: Floor " << request.floorNumber << "\n";
            continue;
        }

        // Go to pickup floor if not already there
        if (currentFloor != request.floorNumber) {
            elevatorState = DOOR_OPEN;
            {
                std::lock_guard<std::mutex> lock(printMutex);
                std::cout << "[ELEVATOR " << elevatorId << "] Arrived at Pickup Floor " << request.floorNumber << ". Doors opening...\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            elevatorState = DOOR_CLOSED;
            {
                std::lock_guard<std::mutex> lock(printMutex);
                std::cout << "[ELEVATOR " << elevatorId << "] Doors closing...\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            currentFloor = request.floorNumber;
        }

        // Move floor-by-floor to destination
        elevatorState = MOVING;
        {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[ELEVATOR " << elevatorId << "] Moving from Floor " << currentFloor
                      << " to Floor " << request.destination << "\n";
        }
        int step = (request.destination > currentFloor) ? 1 : -1;
        while (currentFloor != request.destination) {
            std::this_thread::sleep_for(std::chrono::seconds(FLOOR_TRAVEL_TIME));
            updateTime(currentTime.load() + 1);
            currentFloor += step;
            
            // Send intermediate update (msgType = 3)
            ElevatorMessage updateMsg;
            updateMsg.floorNumber = currentFloor;
            updateMsg.destination = request.destination;
            updateMsg.assignedElevator = elevatorId;
            updateMsg.msgType = 3;
            updateMsg.timestamp = currentTime.load();
            sendto(sockfd, &updateMsg, sizeof(updateMsg), 0, (struct sockaddr*)&schedulerAddr, addrLen);

            {
                std::lock_guard<std::mutex> lock(printMutex);
                std::cout << "[ELEVATOR " << elevatorId << "] Intermediate update: now at Floor " << currentFloor 
                          << " (time " << currentTime.load() << ")\n";
            }
        }

        // At destination: open and close doors
        elevatorState = DOOR_OPEN;
        {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[ELEVATOR " << elevatorId << "] Arrived at Destination Floor " << currentFloor << ". Doors opening...\n";
            }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        elevatorState = DOOR_CLOSED;
        {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[ELEVATOR " << elevatorId << "] Doors closing...\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Send final completion response (msgType = 1)
        request.status = 1;
        request.msgType = 1;
        request.timestamp = currentTime.load();
        sendto(sockfd, &request, sizeof(request), 0, (struct sockaddr*)&schedulerAddr, addrLen);

        isIdle = true;
    }
    close(sockfd);
}
