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
        std::vector<uint8_t> data(100);
        DatagramPacket receivePacket(data, data.size());

        if (schedulerState == SchedulerState::WAITING_FOR_REQUEST) {
            try {
                receiveSocket.receive(receivePacket);
            } catch (const std::runtime_error& e) {
                std::cerr << "Error receiving: " << e.what() << std::endl;
                continue;
            }

            std::string receivedMsg(reinterpret_cast<const char*>(receivePacket.getData()), receivePacket.getLength());
            std::istringstream iss(receivedMsg);
            int floor, destination;
            iss >> floor >> destination;

            std::cout << "Scheduler: Received request from Floor " << floor << " to Floor " << destination << std::endl;
            schedulerState = SchedulerState::SELECTING_ELEVATOR;
        }

        if (schedulerState == SchedulerState::SELECTING_ELEVATOR) {
            std::lock_guard<std::mutex> lock(mtx);
            
            int bestElevator = -1;
            int minDistance = INT_MAX;

            for (int i = 0; i < 2; i++) { 
                if (!elevatorBusy[i]) {
                    int distance = abs(elevatorPositions[i] - floor);
                    if (distance < minDistance) {
                        minDistance = distance;
                        bestElevator = i;
                    }
                }
            }

            if (bestElevator == -1) {
                std::cout << "Scheduler: All elevators busy, waiting..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                continue;
            }

            elevatorBusy[bestElevator] = true;
            schedulerState = SchedulerState::SENDING_TO_ELEVATOR;

            std::string elevatorRequest = std::to_string(bestElevator) + " " + std::to_string(floor) + " " + std::to_string(destination);
            std::vector<uint8_t> out(elevatorRequest.begin(), elevatorRequest.end());
            DatagramPacket sendPacket(out, out.size(), InetAddress::getLocalHost(), ELEVATOR_PORT);

            try {
                sendSocket.send(sendPacket);
                std::cout << "Scheduler: Assigned request to Elevator " << bestElevator << std::endl;
                schedulerState = SchedulerState::WAITING_FOR_ELEVATOR;
            } catch (const std::runtime_error& e) {
                std::cerr << "Error sending to Elevator: " << e.what() << std::endl;
                schedulerState = SchedulerState::WAITING_FOR_REQUEST;
            }
        }

        if (schedulerState == SchedulerState::WAITING_FOR_ELEVATOR) {
            std::vector<uint8_t> data(100);
            DatagramPacket receivePacket(data, data.size());

            try {
                receiveSocket.receive(receivePacket);
            } catch (const std::runtime_error& e) {
                std::cerr << "Error receiving elevator status: " << e.what() << std::endl;
                continue;
            }

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
                schedulerState = SchedulerState::WAITING_FOR_REQUEST;
            }
        }
    }
}
