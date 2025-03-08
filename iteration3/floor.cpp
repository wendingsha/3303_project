#include "Datagram.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <thread>
#include <chrono>

#define SCHEDULER_PORT 9001

void floorFunction() {
    DatagramSocket sendSocket;
    std::ifstream inputFile("input.txt");

    if (!inputFile) {
        std::cerr << "Error: Unable to open input.txt. Exiting floorFunction." << std::endl;
        return;
    }

    std::cout<<"Floor: Reading requests from input.txt..."<<std::endl;

    int floor, destination;
    while (inputFile >> floor >> destination) {
        if (floor == destination) continue; 

        std::string request = std::to_string(floor) + " " + std::to_string(destination);
        std::vector<uint8_t> out(request.begin(), request.end());
        DatagramPacket sendPacket(out, out.size(), InetAddress::getLocalHost(), SCHEDULER_PORT);

        try {
            sendSocket.send(sendPacket);
            std::cout << "Floor: Sent request " << request << " to Scheduler" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } catch (const std::runtime_error& e) {
            std::cerr << "Error sending request: " << e.what() << std::endl;
        }
    }

    std::cout << "Floor: All requests sent. Waiting before exit..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
}

int main() {
    floorFunction();
    return 0;
}

