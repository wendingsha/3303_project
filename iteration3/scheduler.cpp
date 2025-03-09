#include "Datagram.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <chrono>
#include <climits>
#include <queue>

#define SCHEDULER_PORT 9001
#define ELEVATOR_PORT 9002

// 存储楼层请求的结构
struct FloorRequest {
    int startFloor;
    int destinationFloor;
};

std::mutex mtx;
std::unordered_map<int, bool> elevatorBusy = {{0, false}, {1, false}}; 
std::unordered_map<int, int> elevatorPositions = {{0, 0}, {1, 5}}; 
std::queue<FloorRequest> pendingRequests;

// 处理来自楼层的请求线程
void handleFloorRequests() {
    DatagramSocket receiveSocket(SCHEDULER_PORT);
    std::cout << "Scheduler: Listening for floor requests on port " << SCHEDULER_PORT << "..." << std::endl;
    
    while (true) {
        std::vector<uint8_t> data(100);
        DatagramPacket receivePacket(data, data.size());
        
        try {
            std::cout << "Scheduler: Waiting for floor requests..." << std::endl;
            receiveSocket.receive(receivePacket);
            
            std::string receivedMsg(reinterpret_cast<const char*>(receivePacket.getData()), receivePacket.getLength());
            std::istringstream iss(receivedMsg);
            int floor, destination;
            iss >> floor >> destination;
            
            std::cout << "Scheduler: Received request from Floor " << floor << " to Floor " << destination << std::endl;
            
            // 添加到待处理队列
            std::lock_guard<std::mutex> lock(mtx);
            pendingRequests.push({floor, destination});
            
        } catch (const std::runtime_error& e) {
            std::cerr << "Error receiving floor request: " << e.what() << std::endl;
        }
    }
}

// 处理来自电梯的状态更新线程
void handleElevatorUpdates() {
    DatagramSocket receiveSocket(SCHEDULER_PORT + 100); // 使用不同端口9101
    std::cout << "Scheduler: Listening for elevator updates on port " << (SCHEDULER_PORT + 100) << "..." << std::endl;
    
    while (true) {
        std::vector<uint8_t> data(100);
        DatagramPacket receivePacket(data, data.size());
        
        try {
            receiveSocket.receive(receivePacket);
            
            std::string statusMsg(reinterpret_cast<const char*>(receivePacket.getData()), receivePacket.getLength());
            std::istringstream iss(statusMsg);
            int elevatorId, newPosition;
            std::string status;
            iss >> elevatorId >> newPosition >> status;
            
            if (status == "IDLE") {
                std::cout << "Scheduler: Elevator " << elevatorId << " is now IDLE at Floor " << newPosition << std::endl;
                std::lock_guard<std::mutex> lock(mtx);
                elevatorBusy[elevatorId] = false;
                elevatorPositions[elevatorId] = newPosition;
            }
            
        } catch (const std::runtime_error& e) {
            std::cerr << "Error receiving elevator status: " << e.what() << std::endl;
        }
    }
}

// 分配电梯线程
void assignElevators() {
    DatagramSocket sendSocket;
    
    while (true) {
        FloorRequest request;
        bool hasRequest = false;
        
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (!pendingRequests.empty()) {
                request = pendingRequests.front();
                pendingRequests.pop();
                hasRequest = true;
            }
        }
        
        if (hasRequest) {
            int bestElevator = -1;
            int minDistance = INT_MAX;
            
            {
                std::lock_guard<std::mutex> lock(mtx);
                
                for (int i = 0; i < 2; i++) {
                    if (!elevatorBusy[i]) {
                        int distance = abs(elevatorPositions[i] - request.startFloor);
                        if (distance < minDistance) {
                            minDistance = distance;
                            bestElevator = i;
                        }
                    }
                }
            }
            
            if (bestElevator != -1) {
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    elevatorBusy[bestElevator] = true;
                }
                
                std::string elevatorRequest = std::to_string(bestElevator) + " " + 
                                             std::to_string(request.startFloor) + " " + 
                                             std::to_string(request.destinationFloor);
                                             
                std::vector<uint8_t> out(elevatorRequest.begin(), elevatorRequest.end());
                DatagramPacket sendPacket(out, out.size(), InetAddress::getLocalHost(), ELEVATOR_PORT);
                
                try {
                    sendSocket.send(sendPacket);
                    std::cout << "Scheduler: Assigned request to Elevator " << bestElevator << std::endl;
                } catch (const std::runtime_error& e) {
                    std::cerr << "Error sending to Elevator: " << e.what() << std::endl;
                    // 请求失败，放回队列
                    std::lock_guard<std::mutex> lock(mtx);
                    pendingRequests.push(request);
                    elevatorBusy[bestElevator] = false;
                }
            } else {
                // 没有可用电梯，放回队列
                std::cout << "Scheduler: All elevators busy, waiting..." << std::endl;
                std::lock_guard<std::mutex> lock(mtx);
                pendingRequests.push(request);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        } else {
            // 没有请求，短暂休眠
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

int main() {
    // 启动多线程处理
    std::thread floorThread(handleFloorRequests);
    std::thread elevatorThread(handleElevatorUpdates);
    std::thread assignThread(assignElevators);
    
    // 等待线程结束（实际上不会结束）
    floorThread.join();
    elevatorThread.join();
    assignThread.join();
    
    return 0;
}
