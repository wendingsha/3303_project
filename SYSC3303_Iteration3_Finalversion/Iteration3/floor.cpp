/* floor.cpp */
#include "message.hpp"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <thread>
#include <set>
#include <mutex>

#define SCHEDULER_IP "127.0.0.1"
#define SCHEDULER_PORT 8100

extern std::mutex printMutex;
std::set<std::pair<int, int>> sentRequests; 

void floorFunction() {
    std::vector<std::pair<int, int>> requestList = {{1, 5}, {3, 8}, {6, 2}, {9, 4}, {2, 7}, {8, 1}, {4, 10}};
    int timer = 0;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket\n";
        return;
    }

    struct sockaddr_in schedulerAddr;
    memset(&schedulerAddr, 0, sizeof(schedulerAddr));

    schedulerAddr.sin_family = AF_INET;
    schedulerAddr.sin_port = htons(SCHEDULER_PORT);
    inet_pton(AF_INET, SCHEDULER_IP, &schedulerAddr.sin_addr);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (auto& request : requestList) {
        std::pair<int, int> requestPair = {request.first, request.second};

        if (sentRequests.count(requestPair)) continue;
        sentRequests.insert(requestPair);

        ElevatorMessage msg(request.first, request.second, request.second > request.first, -1, 0);
        msg.timestamp = timer;
        sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr*)&schedulerAddr, sizeof(schedulerAddr));

        {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[FLOOR] Sent request: From " << msg.floorNumber << " to " 
                      << msg.destination << " at time " << timer << std::endl;
        }
        
        timer += 4;
        std::this_thread::sleep_for(std::chrono::seconds(4));
    }

    close(sockfd);
}