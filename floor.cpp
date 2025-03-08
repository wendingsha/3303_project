#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <regex>
#include <iomanip>
#include <algorithm>
#include <ctime>

//----------------------------------------------------------------
// Shared DTO and Helpers
//----------------------------------------------------------------

// Enumeration for direction
enum class Direction { UP, DOWN };

// Convert string to Direction
Direction stringToDirection(const std::string& s) {
    return (s == "UP") ? Direction::UP : Direction::DOWN;
}

// Convert Direction to string
std::string directionToString(Direction d) {
    return (d == Direction::UP) ? "UP" : "DOWN";
}

// A minimal ElevatorRequest DTO (usually defined in a shared module)
struct ElevatorRequest {
    std::chrono::system_clock::time_point timestamp;
    int sourceFloor;
    Direction direction;
    int destinationFloor;

    // Returns a string representation (e.g., "10:15:30 1 UP 5")
    std::string toString() const {
        std::time_t t = std::chrono::system_clock::to_time_t(timestamp);
        std::tm* tmPtr = std::localtime(&t);
        char timeBuf[20];
        std::strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", tmPtr);
        std::ostringstream oss;
        oss << timeBuf << " " << sourceFloor << " " << directionToString(direction)
            << " " << destinationFloor;
        return oss.str();
    }
};

// Helper: Parse a time string "HH:MM:SS.mmm" and return a time_point.
// This implementation uses today's date and overrides the time portion.
std::chrono::system_clock::time_point parseTimestamp(const std::string& timeStr) {
    std::istringstream iss(timeStr);
    std::tm tm = {};
    char dot;
    int hour, minute, second, millis = 0;
    iss >> hour >> std::ws;
    iss.ignore(1); // ignore ':'
    iss >> minute;
    iss.ignore(1); // ignore ':'
    iss >> second;
    iss >> dot; // dot
    iss >> millis;

    // Get current date
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* today = std::localtime(&now_c);
    today->tm_hour = hour;
    today->tm_min = minute;
    today->tm_sec = second;
    std::time_t t = std::mktime(today);
    auto tp = std::chrono::system_clock::from_time_t(t);
    tp += std::chrono::milliseconds(millis);
    return tp;
}

//----------------------------------------------------------------
// FloorComponents Class
//----------------------------------------------------------------
// Represents the physical components on a floor (buttons and sensors)
class FloorComponents {
private:
    bool upButton;
    bool downButton;
    Direction directionLamp;  // Indicates the direction sign shown on the floor
    bool arrivalSensor;       // True if an elevator has arrived

public:
    FloorComponents(Direction initialDirection)
        : upButton(false), downButton(false),
          directionLamp(initialDirection), arrivalSensor(false) {}

    // Returns the status of the button lamp based on the direction
    bool getButtonLampStatus(Direction d) const {
        return (d == Direction::UP) ? upButton : downButton;
    }

    // Toggle the button status for a given direction
    void updateButtonDirectionStatus(Direction d) {
        if (d == Direction::UP) {
            upButton = !upButton;
        } else {
            downButton = !downButton;
        }
    }

    // Update the floor's direction lamp (e.g., set to UP or DOWN)
    void updateDirectionLamp(Direction d) {
        directionLamp = d;
    }

    // Update the arrival sensor (true if elevator present)
    void updateArrivalSensor(bool status) {
        arrivalSensor = status;
    }

    // For demonstration, print the current component statuses.
    void displayStatus() const {
        std::cout << "FloorComponents Status:\n"
                  << "  Up Button: " << (upButton ? "Pressed" : "Not Pressed") << "\n"
                  << "  Down Button: " << (downButton ? "Pressed" : "Not Pressed") << "\n"
                  << "  Direction Lamp: " << directionToString(directionLamp) << "\n"
                  << "  Arrival Sensor: " << (arrivalSensor ? "Elevator Arrived" : "No Elevator") << "\n";
    }
};

//----------------------------------------------------------------
// RPC Stub Class
//----------------------------------------------------------------
// A simple stub to simulate UDP communications.
class RPC {
public:
    void openSocket() {
        std::cout << "[RPC] Socket opened.\n";
    }

    void closeSocket() {
        std::cout << "[RPC] Socket closed.\n";
    }

    // Simulate sending data and receiving a reply.
    std::string floorSendReceive(const std::string& data, int port) {
        std::cout << "[RPC] Sending data to port " << port << ": " << data << "\n";
        // For simulation, just return a dummy reply.
        return "Reply from Scheduler";
    }

    // Simulate sending an acknowledgment.
    void floorAck(const std::string& reply) {
        std::cout << "[RPC] Acknowledging reply: " << reply << "\n";
    }
};

//----------------------------------------------------------------
// Parser Class
//----------------------------------------------------------------
// Responsible for reading a text file and converting each line into an ElevatorRequest.
class Parser {
private:
    std::ifstream input;
    std::string filename;

    // Ensure that the millisecond portion has exactly 3 digits.
    std::string fillTimestampZero(const std::string& timePart) {
        // timePart should be in the form "HH:MM:SS.mmm"
        std::regex re(R"((\d{2}:\d{2}:\d{2})\.(\d{1,3}))");
        std::smatch match;
        if (std::regex_match(timePart, match, re)) {
            std::string timeComponent = match[1];
            std::string millis = match[2];
            while (millis.size() < 3)
                millis += "0";
            return timeComponent + "." + millis;
        }
        return timePart; // return original if pattern doesn't match
    }

    // Parses a single line into an ElevatorRequest.
    // Expected line format: "HH:MM:SS.mmm <sourceFloor> <UP|DOWN> <destinationFloor>"
    ElevatorRequest textParser(const std::string& textRequest) {
        std::istringstream iss(textRequest);
        std::string timeStr, srcStr, dirStr, destStr;
        iss >> timeStr >> srcStr >> dirStr >> destStr;
        // Ensure the timestamp has 3-digit milliseconds.
        timeStr = fillTimestampZero(timeStr);
        auto ts = parseTimestamp(timeStr);
        int sourceFloor = std::stoi(srcStr);
        int destinationFloor = std::stoi(destStr);
        Direction direction = stringToDirection(dirStr);
        return ElevatorRequest{ts, sourceFloor, direction, destinationFloor};
    }

public:
    Parser(const std::string& file) : filename(file) {}

    // Reads the file, parses each line into an ElevatorRequest, and returns a vector.
    std::vector<ElevatorRequest> requestParser() {
        std::vector<ElevatorRequest> requests;
        input.open(filename);
        if (!input.is_open()) {
            std::cerr << "Error: Could not open file " << filename << "\n";
            return requests;
        }
        std::string line;
        while (std::getline(input, line)) {
            if (line.empty())
                continue;
            try {
                auto req = textParser(line);
                requests.push_back(req);
            } catch (const std::exception& e) {
                std::cerr << "Error parsing line: " << line << " (" << e.what() << ")\n";
            }
        }
        input.close();
        // Sort the requests by timestamp.
        std::sort(requests.begin(), requests.end(),
                  [](const ElevatorRequest& a, const ElevatorRequest& b) {
                      return a.timestamp < b.timestamp;
                  });
        return requests;
    }
};

//----------------------------------------------------------------
// Floor Class
//----------------------------------------------------------------
// Represents the floor subsystem that reads user requests and sends them to the Scheduler.
class Floor {
private:
    int floorNumber;
    Parser parser;
    RPC rpc;
    FloorComponents components;
    static constexpr int FLOOR_PORT = 23;

    // Encodes an ElevatorRequest as a string (here, simply using toString()).
    std::string encodeData(const ElevatorRequest& req) {
        return req.toString();
    }

    // Simulate sending all parsed requests to the Scheduler.
    void addRequestToQueue(const std::vector<ElevatorRequest>& requests) {
        if (requests.empty()) {
            std::cout << "No elevator requests found.\n";
            return;
        }
        for (const auto& req : requests) {
            std::string data = encodeData(req);
            std::string reply = rpc.floorSendReceive(data, FLOOR_PORT);
            rpc.floorAck(reply);
            std::cout << "--------------------------------------\n";
        }
    }

public:
    // Constructor takes the floor number and the path to the input file.
    Floor(int floorNumber, const std::string& inputFile)
        : floorNumber(floorNumber), parser(inputFile),
          components(Direction::UP) // default initial lamp direction
    {}

    int getFloorNumber() const {
        return floorNumber;
    }

    // Main function to run the floor subsystem.
    void run() {
        rpc.openSocket();
        auto requests = parser.requestParser();
        addRequestToQueue(requests);
        rpc.closeSocket();
        // Display floor components status (for demonstration)
        components.displayStatus();
    }
};

//----------------------------------------------------------------
// Main Entry Point
//----------------------------------------------------------------

int main() {
    // For demonstration, we assume the input file is named "input.txt" in the current directory.
    // Each line in "input.txt" should be formatted as:
    // HH:MM:SS.mmm <sourceFloor> <UP|DOWN> <destinationFloor>
    Floor floorSubsystem(1, "input.txt");
    floorSubsystem.run();
    return 0;
}
