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

extern int currentTime;
extern std::mutex printMutex;
extern bool systemActive;

#define ELEVATOR_PORT_BASE 9100
#define SCHEDULER_IP "127.0.0.1"
#define SCHEDULER_PORT 8100
#define FLOOR_TRAVEL_TIME 1.0
#define MIN_FLOOR 1

void elevatorFunction(int elevatorId) {
    ElevatorState elevatorState = ElevatorState::IDLE;

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

        if (request.floorNumber == request.destination) {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[ELEVATOR " << elevatorId << "] Ignoring same-floor request: Floor " << request.floorNumber << std::endl;
            continue;
        }

        int floorsToTravel = std::abs(request.destination - currentFloor);
        int travelTime = floorsToTravel * FLOOR_TRAVEL_TIME;

       
        if (currentFloor != request.floorNumber) {
            elevatorState = ElevatorState::DOOR_OPEN;
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[ELEVATOR " << elevatorId << "] Arrived at Pickup Floor " << request.floorNumber << ". Doors opening...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

           
            elevatorState = ElevatorState::DOOR_CLOSED;
            std::cout << "[ELEVATOR " << elevatorId << "] Doors closing...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            currentFloor = request.floorNumber;  // Update current position
        }

        // Move to destination
        elevatorState = ElevatorState::MOVING;
        std::lock_guard<std::mutex> lock(printMutex);
        std::cout << "[ELEVATOR " << elevatorId << "] Moving from Floor " << currentFloor
                  << " to Floor " << request.destination << " (Travel time: " << travelTime << "s)\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(travelTime * 1000)));

        currentTime += travelTime;
        currentFloor = request.destination;

        // Open doors at destination floor
        elevatorState = ElevatorState::DOOR_OPEN;
        std::cout << "[ELEVATOR " << elevatorId << "] Arrived at Destination Floor " << currentFloor << ". Doors opening...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Close doors after drop-off
        elevatorState = ElevatorState::DOOR_CLOSED;
        std::cout << "[ELEVATOR " << elevatorId << "] Doors closing...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        request.status = 1;
        request.timestamp = currentTime;
        sendto(sockfd, &request, sizeof(request), 0, (struct sockaddr*)&schedulerAddr, addrLen);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        isIdle = true;
        elevatorState = ElevatorState::IDLE;
        sendto(sockfd, &isIdle, sizeof(isIdle), 0, (struct sockaddr*)&schedulerAddr, addrLen);
    }
    close(sockfd);
}
