#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <chrono>
#include <thread>

//----------------------------------------------------------------
// Minimal stubs for external classes (DTOs, RPC, Scheduler)
// These are provided elsewhere in your project.
//----------------------------------------------------------------

// A minimal stub for ElevatorRequest (DTO) used by the elevator subsystem.
class ElevatorRequest {
public:
    int sourceFloor;
    int destinationFloor;
    std::string direction; // "UP" or "DOWN"

    ElevatorRequest(int src, const std::string &dir, int dest)
        : sourceFloor(src), destinationFloor(dest), direction(dir) {}
};

// A minimal stub for RPC (handles UDP communications).
class RPC {
public:
    static constexpr int ELEVATOR_PORT = 69;

    void openSocket() {
        std::cout << "[RPC] Socket opened.\n";
    }

    void closeSocket() {
        std::cout << "[RPC] Socket closed.\n";
    }

    // Dummy send/receive function.
    std::string elevatorSendReceive(int port) {
        std::cout << "[RPC] Sending/receiving on port " << port << ".\n";
        return "Dummy reply";  // In a full implementation, reply data would be processed.
    }

    // Dummy acknowledgment method.
    void elevatorAck() {
        std::cout << "[RPC] Acknowledgment sent.\n";
    }
};

// A minimal stub for Scheduler (used for logging elevator movement).
class Scheduler {
public:
    void registerElevatorLocation(int id, int floor) {
        std::cout << "[Scheduler] Registered Elevator " << id << " at floor " << floor << ".\n";
    }

    void movingTo(int id, int fromFloor, int toFloor) {
        std::cout << "[Scheduler] Elevator " << id << " moving from floor " << fromFloor << " to floor " << toFloor << ".\n";
    }

    int displayElevatorLocation(int id) {
        // For demonstration purposes, always return floor 1.
        return 1;
    }
};

//----------------------------------------------------------------
// Elevator Subsystem
//----------------------------------------------------------------

// ElevatorState enum and helper functions.
enum class ElevatorState {
    Idle,
    AwaitRequest,
    Moving,
    Stop,
    DoorsOpen,
    DoorsClose
};

ElevatorState nextState(ElevatorState state) {
    switch (state) {
        case ElevatorState::Idle:         return ElevatorState::AwaitRequest;
        case ElevatorState::AwaitRequest: return ElevatorState::Moving;
        case ElevatorState::Moving:       return ElevatorState::Stop;
        case ElevatorState::Stop:         return ElevatorState::DoorsOpen;
        case ElevatorState::DoorsOpen:    return ElevatorState::DoorsClose;
        case ElevatorState::DoorsClose:   return ElevatorState::AwaitRequest;
        default:                          return ElevatorState::Idle;
    }
}

std::string stateToString(ElevatorState state) {
    switch (state) {
        case ElevatorState::Idle:         return "Idle";
        case ElevatorState::AwaitRequest: return "AwaitRequest";
        case ElevatorState::Moving:       return "Moving";
        case ElevatorState::Stop:         return "Stop";
        case ElevatorState::DoorsOpen:    return "DoorsOpen";
        case ElevatorState::DoorsClose:   return "DoorsClose";
        default:                          return "Unknown";
    }
}

std::string displayState(ElevatorState state, int id, const ElevatorRequest* request = nullptr) {
    std::ostringstream oss;
    if (state == ElevatorState::Moving && request != nullptr) {
        oss << "Elevator# " << id << " State: " << stateToString(state)
            << " > Direction: " << request->direction
            << " from floor " << request->sourceFloor
            << " -> floor " << request->destinationFloor;
    } else if (state == ElevatorState::Stop && request != nullptr) {
        oss << "Elevator# " << id << " State: " << stateToString(state)
            << " arrived at floor " << request->destinationFloor;
    } else {
        oss << "Elevator# " << id << " State: " << stateToString(state);
    }
    return oss.str();
}

// ElevatorComponents: Manages elevator button/lamp statuses and mechanical state.
class ElevatorComponents {
public:
    std::map<int, bool> elevatorButtonBoard; // Floor number -> lamp status.
    bool motor;    // true if motor is running.
    bool doorOpen; // true if door is open.

    ElevatorComponents(int totalFloors, bool motor = false, bool doorOpen = false)
        : motor(motor), doorOpen(doorOpen) {
        for (int i = 1; i <= totalFloors; i++) {
            elevatorButtonBoard[i] = false;
        }
    }

    // Toggle the lamp status for a given floor button.
    void updateElevatorLampLight(int buttonNumber) {
        if (elevatorButtonBoard.find(buttonNumber) != elevatorButtonBoard.end()) {
            elevatorButtonBoard[buttonNumber] = !elevatorButtonBoard[buttonNumber];
        }
    }

    // Get the lamp status for a specific floor.
    bool getElevatorLampStatus(int buttonNumber) {
        return (elevatorButtonBoard.find(buttonNumber) != elevatorButtonBoard.end())
                   ? elevatorButtonBoard[buttonNumber]
                   : false;
    }

    // Return a map of all floors where the lamp is on.
    std::map<int, bool> getAllSelectedFloors() {
        std::map<int, bool> selected;
        for (const auto &p : elevatorButtonBoard) {
            if (p.second)
                selected[p.first] = true;
        }
        return selected;
    }

    // Toggle the motor state.
    void updateMotor() {
        motor = !motor;
    }

    // Toggle the door state.
    void updateDoorOpen() {
        doorOpen = !doorOpen;
    }
};

// Elevator class: Implements the elevator subsystem behavior.
class Elevator {
public:
    int id;
    ElevatorState state;
    RPC rpc;
    Scheduler scheduler;
    ElevatorComponents components;
    int currentFloor;

    Elevator(int id, int totalFloors)
        : id(id), state(ElevatorState::Idle),
          components(totalFloors), currentFloor(1) {
        // On startup, register the elevator's location.
        scheduler.registerElevatorLocation(id, currentFloor);
    }

    // Dummy function to simulate decoding an elevator request from received data.
    ElevatorRequest decodeData(const std::string &data) {
        // For demonstration, we return a dummy request.
        return ElevatorRequest(1, "UP", 5);
    }

    // Main loop for the elevator subsystem.
    void run() {
        rpc.openSocket();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // For demonstration, create a dummy request.
        ElevatorRequest request(1, "UP", 5);

        while (true) {
            std::cout << displayState(state, id, &request) << std::endl;
            switch (state) {
                case ElevatorState::Idle:
                    state = nextState(state);
                    break;

                case ElevatorState::AwaitRequest: {
                    std::string reply = rpc.elevatorSendReceive(RPC::ELEVATOR_PORT);
                    // Decode a request (dummy implementation).
                    request = decodeData(reply);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    state = nextState(state);
                    break;
                }

                case ElevatorState::Moving:
                    // Simulate moving: first to the pickup floor then to the destination.
                    if (currentFloor != request.sourceFloor) {
                        std::cout << "Elevator " << id << " moving to floor " << request.sourceFloor << " to pick up passengers." << std::endl;
                        scheduler.movingTo(id, currentFloor, request.sourceFloor);
                        currentFloor = request.sourceFloor;
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    std::cout << "Elevator " << id << " moving to floor " << request.destinationFloor << "." << std::endl;
                    scheduler.movingTo(id, currentFloor, request.destinationFloor);
                    currentFloor = request.destinationFloor;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    state = nextState(state);
                    break;

                case ElevatorState::Stop:
                    rpc.elevatorAck();
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    state = nextState(state);
                    break;

                case ElevatorState::DoorsOpen:
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    state = nextState(state);
                    break;

                case ElevatorState::DoorsClose:
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    state = nextState(state);
                    break;
            }
        }
        rpc.closeSocket();
    }
};

int main() {
    // For demonstration, assume the building has 10 floors.
    Elevator elevator(1, 10);
    elevator.run();
    return 0;
}
