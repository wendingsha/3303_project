#include "Datagram.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <chrono>
#include <climits>

#define SCHEDULER_PORT 9001
#define ELEVATOR_PORT 9002

enum class SchedulerState {
    WAITING_FOR_REQUEST,
    SELECTING_ELEVATOR,
    SENDING_TO_ELEVATOR,
    WAITING_FOR_ELEVATOR
};

std::mutex mtx;
std::unordered_map<int, bool> elevatorBusy = {{0, false}, {1, false}}; 
std::unordered_map<int, int> elevatorPositions = {{0, 0}, {1, 5}}; 
SchedulerState schedulerState = SchedulerState::WAITING_FOR_REQUEST;

void schedulerFunction() {
    DatagramSocket receiveSocket(SCHEDULER_PORT);
    DatagramSocket sendSocket;
    std::cout << "Scheduler: Listening on port " << SCHEDULER_PORT << "..." << std::endl;

    while (true) {
        if (schedulerState == SchedulerState::WAITING_FOR_REQUEST) {
            std::vector<uint8_t> data(100);
            DatagramPacket receivePacket(data, data.size());
            
            std::cout << "Scheduler: Waiting for new request..." << std::endl;
            try {
                receiveSocket.receive(receivePacket);
                std::cout << "Scheduler: Received data of length: " << receivePacket.getLength() << std::endl;
                
                std::string receivedMsg(reinterpret_cast<const char*>(receivePacket.getData()), receivePacket.getLength());
                std::cout << "Scheduler: Raw received message: '" << receivedMsg << "'" << std::endl;
                
                std::istringstream iss(receivedMsg);
                int floor, destination;
                if (iss >> floor >> destination) {
                    std::cout << "Scheduler: Received request from Floor " << floor << " to Floor " << destination << std::endl;
                    schedulerState = SchedulerState::SELECTING_ELEVATOR;
                } else {
                    std::cerr << "Scheduler: Failed to parse floor request format" << std::endl;
                    schedulerState = SchedulerState::WAITING_FOR_REQUEST;
                }
            } catch (const std::runtime_error& e) {
                std::cerr << "Error receiving: " << e.what() << std::endl;
                continue;
            }
        }

        // 其余代码保持不变...
        // 处理SELECTING_ELEVATOR, SENDING_TO_ELEVATOR, WAITING_FOR_ELEVATOR状态
        
        // 在其他状态下如果长时间没有响应，考虑重置状态
        if (schedulerState != SchedulerState::WAITING_FOR_REQUEST) {
            // 可以添加超时逻辑，例如：
            // if (超时) schedulerState = SchedulerState::WAITING_FOR_REQUEST;
        }
    }
}

int main() {
    schedulerFunction();
    return 0;
}
