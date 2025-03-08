#include "Datagram.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <cstdlib>

#define SCHEDULER_PORT 9001
#define ELEVATOR_IP "127.0.0.1"
#define ELEVATOR_PORT 9002

std::unordered_map<int, bool> elevatorBusy; 

void schedulerFunction() {
    DatagramSocket receiveSocket(SCHEDULER_PORT);
    DatagramSocket sendSocket;
    std::cout << "Scheduler: Listening on port " << SCHEDULER_PORT << "..." << std::endl;

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
        int floor, destination;
        iss >> floor >> destination;

        std::cout << "Scheduler: Received request from Floor " << floor << " to Floor " << destination << std::endl;

        int assignedElevator = 1;
        elevatorBusy[assignedElevator] = true;

        std::string elevatorRequest = std::to_string(assignedElevator) + " " + std::to_string(floor) + " " + std::to_string(destination);
        std::vector<uint8_t> out(elevatorRequest.begin(), elevatorRequest.end());

        DatagramPacket sendPacket(out, out.size(), InetAddress::getLocalHost(), ELEVATOR_PORT);
        try {
            sendSocket.send(sendPacket);
            std::cout << "Scheduler: Assigned request to Elevator " << assignedElevator << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << "Error sending to Elevator: " << e.what() << std::endl;
        }
    }
}
