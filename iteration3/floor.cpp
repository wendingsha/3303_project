#include "Datagram.h"
#include <iostream>
#include <vector>
#include <fstream>

#define SCHEDULER_IP "127.0.0.1"
#define SCHEDULER_PORT 9001

void floorFunction() {
    DatagramSocket sendSocket;
    std::ifstream inputFile("input.txt");

    if (!inputFile) {
        std::cerr << "Error: Unable to open input.txt. Exiting floorFunction." << std::endl;
        return;
    }

    int floor, destination;
    while (inputFile >> floor >> destination) {
        if (floor == destination) continue;  // 忽略无效请求

        std::string request = std::to_string(floor) + " " + std::to_string(destination);
        std::vector<uint8_t> out(request.begin(), request.end());
        DatagramPacket sendPacket(out, out.size(), InetAddress::getLocalHost(), SCHEDULER_PORT);

        try {
            sendSocket.send(sendPacket);
            std::cout << "Floor: Sent request " << request << " to Scheduler" << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << "Error sending request: " << e.what() << std::endl;
        }
    }
}
