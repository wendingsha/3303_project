#include "Datagram.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <thread>
#include <chrono>

#define ELEVATOR_PORT 9002
#define SCHEDULER_PORT 9001

enum class ElevatorState { IDLE, MOVING, STOPPING, DOOR_OPEN, DOOR_CLOSED };

void elevatorFunction(int elevatorId) {
    DatagramSocket receiveSocket(ELEVATOR_PORT);
    DatagramSocket sendSocket;
    ElevatorState state = ElevatorState::IDLE;

    std::cout << "Elevator " << elevatorId << " Listening on port " << ELEVATOR_PORT << "..." << std::endl;

    while (true) {
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

        // After moving, it will stop and open doors
        state = ElevatorState::STOPPING;
        std::cout << "Elevator " << elevatorId << " stopping at floor " << destinationFloor << std::endl;

        state = ElevatorState::DOOR_OPEN;
        std::cout << "Elevator " << elevatorId << " opening doors..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Doors open for 1 second

        state = ElevatorState::DOOR_CLOSED;
        std::cout << "Elevator " << elevatorId << " closing doors..." << std::endl;

        std::string updateMsg = std::to_string(elevatorId) + " " + std::to_string(destinationFloor) + " IDLE";
        std::vector<uint8_t> out(updateMsg.begin(), updateMsg.end());
        DatagramPacket sendPacket(out, out.size(), InetAddress::getLocalHost(), SCHEDULER_PORT);
        sendSocket.send(sendPacket);

        std::cout << "Elevator " << elevatorId << " is now idle." << std::endl;
        state = ElevatorState::IDLE;
    }
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./elevator <elevator_id>" << std::endl;
        return 1;
    }

    int elevatorId = std::stoi(argv[1]);
    elevatorFunction(elevatorId);
    return 0;
}

