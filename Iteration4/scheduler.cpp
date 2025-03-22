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
#include <thread>

#define SCHEDULER_PORT 8100
#define ELEVATOR_PORT_BASE 9100
#define MAX_ELEVATORS 2
#define MIN_FLOOR 1  
#define RESPONSE_TIMEOUT 10  // seconds

extern bool systemActive;

std::mutex printMutex;
std::mutex pendingMutex;
std::queue<ElevatorMessage> pendingRequests;
std::set<std::pair<int, int>> processedRequests;

int sockfd;

SchedulerState schedulerState = IDLE_SCHEDULER;

// Elevator structure updated with fault flag.
struct Elevator {
    int id;
    int position;
    bool isMoving;
    bool isIdle;
    bool goingUp;
    int passengerCount;
    bool isFaulted; // added to indicate a hard fault (shutdown)
    struct sockaddr_in address;
};

std::vector<Elevator> elevators(MAX_ELEVATORS);

// Structure to track in-progress assignments.
struct InProgressRequest {
    ElevatorMessage msg;
    int assignedTime;
    int elevatorId;
};

std::mutex inProgressMutex;
std::vector<InProgressRequest> inProgressRequests;

// Fault monitor thread: if a request does not get a response within RESPONSE_TIMEOUT, mark the elevator as faulted.
void faultMonitor() {
    while(systemActive) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::lock_guard<std::mutex> lock(inProgressMutex);
        for (auto it = inProgressRequests.begin(); it != inProgressRequests.end(); ) {
            if (currentTime.load() - it->assignedTime > RESPONSE_TIMEOUT) {
                {
                    std::lock_guard<std::mutex> lock(printMutex);
                    std::cout << "[SCHEDULER] HARD FAULT: Elevator " << it->elevatorId 
                              << " did not respond in time for request from Floor " 
                              << it->msg.floorNumber << " to " << it->msg.destination << "\n";
                }
                // Mark this elevator as faulted (shutdown it) and do not assign it further.
                elevators[it->elevatorId].isFaulted = true;
                elevators[it->elevatorId].isIdle = false;
                elevators[it->elevatorId].isMoving = false;
                // Requeue the request for reassignment.
                {
                    std::lock_guard<std::mutex> pendingLock(pendingMutex);
                    pendingRequests.push(it->msg);
                }
                it = inProgressRequests.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void assignElevator() {
    schedulerState = ASSIGNING;
    
    pendingMutex.lock();
    if (pendingRequests.empty()) {
        pendingMutex.unlock();
        return;
    }
    ElevatorMessage request = pendingRequests.front();
    pendingRequests.pop();
    pendingMutex.unlock();

    if (request.floorNumber == request.destination || request.destination < MIN_FLOOR) {
        std::lock_guard<std::mutex> lock(printMutex);
        std::cout << "[SCHEDULER] Ignoring invalid request: From " << request.floorNumber 
                  << " to " << request.destination << "\n";
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

    // Choose an elevator that is not faulted.
    for (auto &elevator : elevators) {
        if (elevator.isFaulted) continue;
        if (elevator.isIdle && elevator.position == request.floorNumber) {
            bestElevator = elevator.id;
            break;
        }
    }
    if (bestElevator == -1) {
        for (auto &elevator : elevators) {
            if (elevator.isFaulted) continue;
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
    if (bestElevator == -1) {
        for (auto &elevator : elevators) {
            if (elevator.isFaulted) continue;
            if (elevator.isIdle) {
                int distance = std::abs(elevator.position - request.floorNumber);
                if (distance < minDistance) {
                    minDistance = distance;
                    bestElevator = elevator.id;
                }
            }
        }
    }
    if (bestElevator == -1) {
        for (auto &elevator : elevators) {
            if (elevator.isFaulted) continue;
            if (elevator.passengerCount < minStops) {
                minStops = elevator.passengerCount;
                bestElevator = elevator.id;
            }
        }
    }

    if (bestElevator == -1) {
        std::lock_guard<std::mutex> lock(printMutex);
        std::cerr << "[SCHEDULER] ERROR: No available (non-faulted) elevator for request from " 
                  << request.floorNumber << " to " << request.destination << "!\n";
        return;
    }

    request.assignedElevator = bestElevator;
    request.msgType = 0;  // assignment message
    elevators[bestElevator].isIdle = false;
    elevators[bestElevator].isMoving = true;
    elevators[bestElevator].goingUp = request.directionUp;
    elevators[bestElevator].passengerCount++;

    {
        std::lock_guard<std::mutex> lock(printMutex);
        std::cout << "[SCHEDULER] Assigned request (From " << request.floorNumber 
                  << " to " << request.destination << ") to Elevator " << bestElevator 
                  << " at time " << request.timestamp << "\n";
    }

    sendto(sockfd, &request, sizeof(request), 0, (struct sockaddr*)&elevators[bestElevator].address, sizeof(elevators[bestElevator].address));

    {
        std::lock_guard<std::mutex> lock(inProgressMutex);
        InProgressRequest ipr;
        ipr.msg = request;
        ipr.assignedTime = currentTime.load();
        ipr.elevatorId = bestElevator;
        inProgressRequests.push_back(ipr);
    }
    schedulerState = IDLE_SCHEDULER;
}

void schedulerFunction() {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket\n";
        return;
    }

    struct sockaddr_in schedulerAddr, senderAddr;
    memset(&schedulerAddr, 0, sizeof(schedulerAddr));
    memset(&senderAddr, 0, sizeof(senderAddr));

    schedulerAddr.sin_family = AF_INET;
    schedulerAddr.sin_addr.s_addr = INADDR_ANY;
    schedulerAddr.sin_port = htons(SCHEDULER_PORT);

    if (bind(sockfd, (struct sockaddr*)&schedulerAddr, sizeof(schedulerAddr)) < 0) {
        std::cerr << "Bind failed\n";
        return;
    }

    {
        std::lock_guard<std::mutex> lock(printMutex);
        std::cout << "[SCHEDULER] Listening for requests and elevator responses...\n";
    }

    // Initialize elevator info.
    for (int i = 0; i < MAX_ELEVATORS; i++) {
        elevators[i].id = i;
        elevators[i].position = MIN_FLOOR;
        elevators[i].isMoving = false;
        elevators[i].isIdle = true;
        elevators[i].goingUp = true;
        elevators[i].passengerCount = 0;
        elevators[i].isFaulted = false;
        elevators[i].address.sin_family = AF_INET;
        elevators[i].address.sin_port = htons(ELEVATOR_PORT_BASE + i);
        inet_pton(AF_INET, "127.0.0.1", &elevators[i].address.sin_addr);
    }

    // Start fault monitor thread.
    std::thread faultThread(faultMonitor);

    ElevatorMessage request;
    socklen_t addrLen = sizeof(senderAddr);

    while (systemActive) {
        memset(&request, 0, sizeof(request));
        int recvResult = recvfrom(sockfd, &request, sizeof(request), 0, (struct sockaddr*)&senderAddr, &addrLen);
        if (recvResult <= 0)
            continue;
        updateTime(request.timestamp);

        if (request.msgType == 0) {
            // New request from floor subsystem.
            {
                std::lock_guard<std::mutex> lock(pendingMutex);
                pendingRequests.push(request);
            }
            assignElevator();
        } else if (request.msgType == 1) {
            // Normal completion response.
            {
                std::lock_guard<std::mutex> lock(inProgressMutex);
                for (auto it = inProgressRequests.begin(); it != inProgressRequests.end(); ++it) {
                    if (it->msg.floorNumber == request.floorNumber &&
                        it->msg.destination == request.destination &&
                        it->elevatorId == request.assignedElevator) {
                        inProgressRequests.erase(it);
                        break;
                    }
                }
            }
            int eid = request.assignedElevator;
            // Only update if elevator is not faulted.
            if (!elevators[eid].isFaulted) {
                elevators[eid].position = request.destination;
                elevators[eid].isIdle = true;
                elevators[eid].isMoving = false;
            }
            {
                std::lock_guard<std::mutex> lock(printMutex);
                std::cout << "[SCHEDULER] Received completion response from Elevator " << eid << "\n";
            }
        } else if (request.msgType == 2) {
            // Fault response from an elevator (transient fault).
            {
                std::lock_guard<std::mutex> lock(printMutex);
                std::cout << "[SCHEDULER] Received fault report from Elevator " << request.assignedElevator
                          << " for request from Floor " << request.floorNumber << " to " << request.destination << "\n";
            }
            int eid = request.assignedElevator;
            // For transient faults, we mark the elevator as idle (and possibly try again).
            if (!elevators[eid].isFaulted) {
                elevators[eid].isIdle = true;
                elevators[eid].isMoving = false;
            }
            {
                std::lock_guard<std::mutex> lock(pendingMutex);
                pendingRequests.push(request);
            }
            {
                std::lock_guard<std::mutex> lock(inProgressMutex);
                for (auto it = inProgressRequests.begin(); it != inProgressRequests.end(); ++it) {
                    if (it->elevatorId == eid &&
                        it->msg.floorNumber == request.floorNumber &&
                        it->msg.destination == request.destination) {
                        inProgressRequests.erase(it);
                        break;
                    }
                }
            }
            assignElevator();
        } else if (request.msgType == 3) {
            // Intermediate update.
            int eid = request.assignedElevator;
            if (!elevators[eid].isFaulted) {
                elevators[eid].position = request.floorNumber;
            }
            {
                std::lock_guard<std::mutex> lock(printMutex);
                std::cout << "[SCHEDULER] Intermediate update: Elevator " << eid 
                          << " is now at Floor " << request.floorNumber 
                          << " (time " << request.timestamp << ")\n";
            }
        }
    }
    faultThread.join();
    close(sockfd);
}
