/* floor.cpp */
#include "message.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <mutex>
#include <thread>
#include <random>

#define SCHEDULER_IP "127.0.0.1"
#define SCHEDULER_PORT 8100
#define INPUT_FILE "input.txt"
#define MIN_FLOOR 1
#define MAX_FLOOR 12

extern std::mutex printMutex;

void floorFunction() {
    int timer = 0;
    std::ifstream infile(INPUT_FILE);
    if (!infile.is_open()) {
        std::cerr << "[FLOOR] Error opening input file: " << INPUT_FILE << "\n";
        return;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "[FLOOR] Error creating socket\n";
        return;
    }

    struct sockaddr_in schedulerAddr;
    memset(&schedulerAddr, 0, sizeof(schedulerAddr));
    schedulerAddr.sin_family = AF_INET;
    schedulerAddr.sin_port = htons(SCHEDULER_PORT);
    inet_pton(AF_INET, SCHEDULER_IP, &schedulerAddr.sin_addr);

    // Set up random number generator for destination floor.
    std::random_device rd;
    std::mt19937 gen(rd());

    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty()) continue;
        // Expected format: pickup_floor, direction [, faultCode]
        std::istringstream iss(line);
        std::string floorStr, directionStr, faultStr;
        if (!std::getline(iss, floorStr, ',')) continue;
        if (!std::getline(iss, directionStr, ',')) continue;
        // faultStr is optional
        std::getline(iss, faultStr, ',');

        // Trim whitespace.
        floorStr.erase(0, floorStr.find_first_not_of(" \t"));
        floorStr.erase(floorStr.find_last_not_of(" \t") + 1);
        directionStr.erase(0, directionStr.find_first_not_of(" \t"));
        directionStr.erase(directionStr.find_last_not_of(" \t") + 1);
        faultStr.erase(0, faultStr.find_first_not_of(" \t"));
        faultStr.erase(faultStr.find_last_not_of(" \t") + 1);

        int pickupFloor = std::stoi(floorStr);
        bool directionUp = (directionStr == "UP" || directionStr == "up");

        // If at boundary, flip direction.
        if (pickupFloor == MIN_FLOOR && !directionUp) {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[FLOOR] Request at MIN floor " << pickupFloor << " with DOWN, flipping to UP\n";
            directionUp = true;
        }
        if (pickupFloor == MAX_FLOOR && directionUp) {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[FLOOR] Request at MAX floor " << pickupFloor << " with UP, flipping to DOWN\n";
            directionUp = false;
        }

        // Generate destination floor based on direction.
        int destination = pickupFloor;
        if (directionUp) {
            std::uniform_int_distribution<int> dist(pickupFloor + 1, MAX_FLOOR);
            destination = dist(gen);
        } else {
            std::uniform_int_distribution<int> dist(MIN_FLOOR, pickupFloor - 1);
            destination = dist(gen);
        }
        
        // Determine fault code (if provided, else 0).
        int faultCode = 0;
        if (!faultStr.empty()) {
            faultCode = std::stoi(faultStr);
        }

        // Create and send the request message.
        ElevatorMessage msg(pickupFloor, destination, directionUp, -1, timer);
        msg.msgType = 0;  // new floor request
        msg.faultCode = faultCode;
        sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr*)&schedulerAddr, sizeof(schedulerAddr));

        {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[FLOOR] Sent request: Pickup Floor " << pickupFloor 
                      << ", Direction " << (directionUp ? "UP" : "DOWN")
                      << ", Generated Destination " << destination 
                      << ", FaultCode " << faultCode 
                      << " at time " << timer << "\n";
        }
        timer += 4;
        std::this_thread::sleep_for(std::chrono::seconds(4));
    }
    infile.close();
    close(sockfd);
}
