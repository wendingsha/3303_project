#include <iostream>
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>

// UDP Socket headers (POSIX)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

//
// 1. Direction enum
//
enum class Direction {
    UP,
    DOWN
};

//
// 2. ElevatorRequest DTO
//
class ElevatorRequest {
public:
    std::chrono::system_clock::time_point timestamp;
    int sourceFloor;
    Direction direction;
    int destinationFloor;

    // Constructor using a time_point
    ElevatorRequest(const std::chrono::system_clock::time_point& ts, int src, Direction dir, int dest)
        : timestamp(ts), sourceFloor(src), direction(dir), destinationFloor(dest) {}

    // Constructor using a timestamp string in "yyyy-MM-dd HH:mm:ss.SSS" format
    ElevatorRequest(const std::string& timestampStr, int src, Direction dir, int dest)
        : sourceFloor(src), direction(dir), destinationFloor(dest) {
        timestamp = stringToTimestamp(timestampStr);
    }

    // Convert a timestamp string to a time_point
    static std::chrono::system_clock::time_point stringToTimestamp(const std::string& timestampStr) {
        std::tm tm = {};
        std::istringstream ss(timestampStr);
        // Parse up to seconds; expected format: "yyyy-MM-dd HH:mm:ss"
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        if(ss.fail()) {
            throw std::runtime_error("Failed to parse time");
        }
        auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

        // If milliseconds are provided (after a dot), add them
        size_t dotPos = timestampStr.find('.');
        if(dotPos != std::string::npos) {
            std::string msStr = timestampStr.substr(dotPos + 1);
            int ms = std::stoi(msStr);
            tp += std::chrono::milliseconds(ms);
        }
        return tp;
    }

    // Returns a string representation of the request
    std::string toString() const {
        std::time_t t = std::chrono::system_clock::to_time_t(timestamp);
        std::tm tm = *std::localtime(&t);
        char buffer[30];
        std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
        std::ostringstream oss;
        oss << buffer << " " << sourceFloor << " " 
            << (direction == Direction::UP ? "UP" : "DOWN")
            << " " << destinationFloor;
        return oss.str();
    }

    // Equality comparison operator
    bool operator==(const ElevatorRequest& other) const {
        return std::chrono::system_clock::to_time_t(timestamp) == std::chrono::system_clock::to_time_t(other.timestamp)
            && sourceFloor == other.sourceFloor
            && direction == other.direction
            && destinationFloor == other.destinationFloor;
    }
};

//
// 3. RPC class for UDP communication
//
class RPC {
public:
    int dataSocket;
    int ackSocket;
    int floorSocket;
    int elevatorSocket;
    static const int FLOOR_PORT = 23;
    static const int ELEVATOR_PORT = 69;

    RPC() : dataSocket(-1), ackSocket(-1), floorSocket(-1), elevatorSocket(-1) {}

    ~RPC() {
        closeSocket();
        closeSchedulerSocket();
    }

    // Open sockets for sending data and acks
    void openSocket() {
        dataSocket = socket(AF_INET, SOCK_DGRAM, 0);
        ackSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if(dataSocket < 0 || ackSocket < 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }
    }

    // Open and bind sockets for the Scheduler
    void openSchedulerSocket() {
        floorSocket = socket(AF_INET, SOCK_DGRAM, 0);
        elevatorSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if(floorSocket < 0 || elevatorSocket < 0) {
            perror("Scheduler socket creation failed");
            exit(EXIT_FAILURE);
        }
        sockaddr_in floorAddr, elevatorAddr;
        memset(&floorAddr, 0, sizeof(floorAddr));
        floorAddr.sin_family = AF_INET;
        floorAddr.sin_addr.s_addr = INADDR_ANY;
        floorAddr.sin_port = htons(FLOOR_PORT);
        if(bind(floorSocket, (struct sockaddr*)&floorAddr, sizeof(floorAddr)) < 0) {
            perror("Bind floorSocket failed");
            exit(EXIT_FAILURE);
        }

        memset(&elevatorAddr, 0, sizeof(elevatorAddr));
        elevatorAddr.sin_family = AF_INET;
        elevatorAddr.sin_addr.s_addr = INADDR_ANY;
        elevatorAddr.sin_port = htons(ELEVATOR_PORT);
        if(bind(elevatorSocket, (struct sockaddr*)&elevatorAddr, sizeof(elevatorAddr)) < 0) {
            perror("Bind elevatorSocket failed");
            exit(EXIT_FAILURE);
        }
    }

    // Close general sockets
    void closeSocket() {
        if(dataSocket != -1) close(dataSocket);
        if(ackSocket != -1) close(ackSocket);
        dataSocket = ackSocket = -1;
    }

    // Close scheduler-specific sockets
    void closeSchedulerSocket() {
        if(floorSocket != -1) close(floorSocket);
        if(elevatorSocket != -1) close(elevatorSocket);
        floorSocket = elevatorSocket = -1;
    }

    // Simplified example: send data and wait for a reply (for the Floor subsystem)
    std::string floorSendReceive(const std::string &data, int port) {
        sockaddr_in destAddr;
        memset(&destAddr, 0, sizeof(destAddr));
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(port);
        destAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if(sendto(dataSocket, data.c_str(), data.size(), 0, (struct sockaddr*)&destAddr, sizeof(destAddr)) < 0) {
            perror("sendto failed");
        }

        char buffer[1024];
        socklen_t addrLen = sizeof(destAddr);
        ssize_t n = recvfrom(dataSocket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&destAddr, &addrLen);
        if(n < 0) {
            perror("recvfrom failed");
        }
        buffer[n] = '\0';
        return std::string(buffer);
    }

    // Additional RPC functions (e.g., ack, elevatorSendReceive, etc.) can be implemented similarly.
};

//
// Main function demonstrating usage of the DTOs in C++
//
int main() {
    try {
        // Create an ElevatorRequest using a timestamp string
        ElevatorRequest req("2023-03-11 10:15:30.123", 1, Direction::UP, 5);
        std::cout << "ElevatorRequest: " << req.toString() << std::endl;
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    // Example usage of the RPC class
    RPC rpc;
    rpc.openSocket();
    // For demonstration purposes, send a message to UDP port 5000 on localhost.
    // Adjust the port/address as needed for your testing.
    std::string response = rpc.floorSendReceive("Test message", 5000);
    std::cout << "Received response: " << response << std::endl;
    rpc.closeSocket();

    return 0;
}
