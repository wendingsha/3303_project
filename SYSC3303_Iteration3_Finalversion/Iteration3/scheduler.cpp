/* scheduler.cpp */
#include "message.hpp"  
#include "time_manager.hpp"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <limits>
#include <set>
#include <queue>
#include <mutex>

#define SCHEDULER_PORT 8100
#define ELEVATOR_PORT_BASE 9100
#define MAX_ELEVATORS 2
#define FLOOR_TRAVEL_TIME 1.0  
#define MIN_FLOOR 1  

extern bool systemActive;



std::mutex printMutex;
std::set<std::pair<int, int>> processedRequests;
std::queue<ElevatorMessage> pendingRequests;
int sockfd;

SchedulerState schedulerState = SchedulerState::IDLE; 

struct Elevator {
    int id;
    int position;
    bool isMoving;
    bool isIdle;
    bool goingUp;
    int passengerCount;
    struct sockaddr_in address;
};

std::vector<Elevator> elevators(MAX_ELEVATORS);

void assignElevator() {
    schedulerState = SchedulerState::ASSIGNING;
    if (pendingRequests.empty()) return;

    ElevatorMessage request = pendingRequests.front();
    pendingRequests.pop();

   
    if (request.floorNumber == request.destination || request.destination < MIN_FLOOR) {
        std::lock_guard<std::mutex> lock(printMutex);
        //std::cout << "[SCHEDULER] Ignoring invalid request: From " << request.floorNumber 
                  //<< " to " << request.destination << std::endl;
        return;
    }

    std::pair<int, int> requestPair = {request.floorNumber, request.destination};

    
    if (processedRequests.count(requestPair)) {
        return;
    }
    processedRequests.insert(requestPair);

    int bestElevator = -1;
    int minDistance = std::numeric_limits<int>::max();
    int minStops = std::numeric_limits<int>::max();

    // Prioritize idle elevators already at the request's floor**
    for (auto &elevator : elevators) {
        if (elevator.isIdle && elevator.position == request.floorNumber) {
            bestElevator = elevator.id;
            break;
        }
    }

    // Prioritize moving elevators heading in the same direction**
    if (bestElevator == -1) {
        for (auto &elevator : elevators) {
            if (elevator.isMoving && 
                ((elevator.goingUp && request.floorNumber >= elevator.position) || 
                 (!elevator.goingUp && request.floorNumber <= elevator.position))) {
                int distance = std::abs(elevator.position - request.floorNumber);
                if (distance < minDistance) {
                    minDistance = distance;
                    bestElevator = elevator.id;
                }
            }
        }
    }

    // If no moving elevator matches, pick the closest idle elevator**
    if (bestElevator == -1) {
        for (auto &elevator : elevators) {
            if (elevator.isIdle) {
                int distance = std::abs(elevator.position - request.floorNumber);
                if (distance < minDistance) {
                    minDistance = distance;
                    bestElevator = elevator.id;
                }
            }
        }
    }

    // Assign to the least busy elevator**
    if (bestElevator == -1) {
        for (auto &elevator : elevators) {
            if (elevator.passengerCount < minStops) {
                minStops = elevator.passengerCount;
                bestElevator = elevator.id;
            }
        }
    }

    // Ensure a valid elevator is assigned**
    if (bestElevator == -1) {
        std::lock_guard<std::mutex> lock(printMutex);
        std::cerr << "[SCHEDULER] ERROR: No elevator available for request from " 
                  << request.floorNumber << " to " << request.destination << "!\n";
        return;  
    }

    request.assignedElevator = bestElevator;
    elevators[bestElevator].isIdle = false;
    elevators[bestElevator].isMoving = true;
    elevators[bestElevator].goingUp = request.directionUp;
    elevators[bestElevator].passengerCount++;

    {
        std::lock_guard<std::mutex> lock(printMutex);
        std::cout << "[SCHEDULER] Assigned request (From " << request.floorNumber 
                  << " to " << request.destination << ") to Elevator " << bestElevator 
                  << " at time " << request.timestamp << std::endl;
    }

    sendto(sockfd, &request, sizeof(request), 0, (struct sockaddr*)&elevators[bestElevator].address, sizeof(elevators[bestElevator].address));
    schedulerState = SchedulerState::IDLE;
}



void schedulerFunction() {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket\n";
        return;
    }

    struct sockaddr_in schedulerAddr, floorAddr;
    memset(&schedulerAddr, 0, sizeof(schedulerAddr));
    memset(&floorAddr, 0, sizeof(floorAddr));

    schedulerAddr.sin_family = AF_INET;
    schedulerAddr.sin_addr.s_addr = INADDR_ANY;
    schedulerAddr.sin_port = htons(SCHEDULER_PORT);

    if (bind(sockfd, (struct sockaddr*)&schedulerAddr, sizeof(schedulerAddr)) < 0) {
        std::cerr << "Bind failed\n";
        return;
    }

    {
        std::lock_guard<std::mutex> lock(printMutex);
        std::cout << "[SCHEDULER] Listening for requests...\n";
    }

    for (int i = 0; i < MAX_ELEVATORS; i++) {
        elevators[i].id = i;
        elevators[i].position = MIN_FLOOR;
        elevators[i].isMoving = false;
        elevators[i].isIdle = true;
        elevators[i].goingUp = true;
        elevators[i].passengerCount = 0;
        elevators[i].address.sin_family = AF_INET;
        elevators[i].address.sin_port = htons(ELEVATOR_PORT_BASE + i);
        inet_pton(AF_INET, "127.0.0.1", &elevators[i].address.sin_addr);
    }

    ElevatorMessage request;
    socklen_t addrLen = sizeof(floorAddr);

    while (systemActive) {
        schedulerState = SchedulerState::PROCESSING;
        memset(&request, 0, sizeof(request));
        recvfrom(sockfd, &request, sizeof(request), 0, (struct sockaddr*)&floorAddr, &addrLen);
        updateTime(request.timestamp);

        pendingRequests.push(request);
        assignElevator();
    }
    close(sockfd);
}
