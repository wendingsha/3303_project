#include "Datagram.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

#define ELEVATOR_PORT 9002
#define SCHEDULER_IP "127.0.0.1"
#define SCHEDULER_PORT 9001

enum class ElevatorState { IDLE, MOVING, STOPPING, DOOR_OPEN, DOOR_CLOSED };

std::mutex mtx;
std::condition_variable cv;
bool systemActive = true;

void elevatorFunction(int elevatorId) {
    DatagramSocket receiveSocket(ELEVATOR_PORT);
    DatagramSocket sendSocket;
    ElevatorState state = ElevatorState::IDLE;

    std::cout << "Elevator: Listening on port " << ELEVATOR_PORT << "..." << std::endl;

    while (systemActive) {
        std::vector<uint8_t> data(100);
        DatagramPacket receivePacket(data, data.size());

        try {
            receiveSocket.receive(receivePacket);
        } catch (const std::runtime_error& e) {
            std::cerr << "Error receiving: " << e.what() << std::endl;
            continue;
        }

        std::string receivedMsg(reinterpret_cast<const char*>(receivePacket.getData()), receivePacket.getLength());
        std::istringstream iss(receivedMsg);
        int receivedElevatorId, pickupFloor, destinationFloor;
        iss >> receivedElevatorId >> pickupFloor >> destinationFloor;

        if (receivedElevatorId != elevatorId) continue; 

        std::cout << "Elevator " << elevatorId << " moving from " << pickupFloor << " to " << destinationFloor << std::endl;

        state = ElevatorState::MOVING;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        state = ElevatorState::STOPPING;
        std::cout << "Elevator " << elevatorId << " stopping at Floor " << pickupFloor << std::endl;
        
        state = ElevatorState::DOOR_OPEN;
        std::cout << "Elevator " << elevatorId << " doors opening at Floor " << pickupFloor << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        state = ElevatorState::DOOR_CLOSED;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        state = ElevatorState::MOVING;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        state = ElevatorState::STOPPING;
        std::cout << "Elevator " << elevatorId << " stopping at Floor " << destinationFloor << std::endl;

        state = ElevatorState::DOOR_OPEN;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        std::string updateMsg = std::to_string(elevatorId) + " IDLE";
        std::vector<uint8_t> out(updateMsg.begin(), updateMsg.end());
        DatagramPacket sendPacket(out, out.size(), InetAddress::getLocalHost(), SCHEDULER_PORT);
        sendSocket.send(sendPacket);

        std::cout << "Elevator " << elevatorId << " is now idle." << std::endl;
    }
}
