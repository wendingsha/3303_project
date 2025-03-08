#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <algorithm>

//----------------------------------------------------------------
// Shared DTO and Helper Functions
//----------------------------------------------------------------

// Direction enum and conversion functions
enum class Direction { UP, DOWN };

std::string directionToString(Direction d) {
    return (d == Direction::UP) ? "UP" : "DOWN";
}

Direction stringToDirection(const std::string &s) {
    return (s == "UP") ? Direction::UP : Direction::DOWN;
}

// Minimal ElevatorRequest DTO
struct ElevatorRequest {
    std::string timestamp; // e.g., "12:00:00.123"
    int sourceFloor;
    Direction direction;
    int destinationFloor;
    
    ElevatorRequest() : timestamp("00:00:00.000"), sourceFloor(0), direction(Direction::UP), destinationFloor(0) {}
    
    ElevatorRequest(const std::string &ts, int src, Direction dir, int dest)
        : timestamp(ts), sourceFloor(src), direction(dir), destinationFloor(dest) {}
        
    std::string toString() const {
        std::ostringstream oss;
        oss << timestamp << " " << sourceFloor << " " << directionToString(direction) << " " << destinationFloor;
        return oss.str();
    }
    
    bool operator==(const ElevatorRequest &other) const {
        return timestamp == other.timestamp &&
               sourceFloor == other.sourceFloor &&
               direction == other.direction &&
               destinationFloor == other.destinationFloor;
    }
};

//----------------------------------------------------------------
// SchedulerState Enum and Next-State Logic
//----------------------------------------------------------------

enum class SchedulerState { Idle, Ready, InService };

SchedulerState nextState(SchedulerState state) {
    switch(state) {
        case SchedulerState::Idle:         return SchedulerState::Ready;
        case SchedulerState::Ready:        return SchedulerState::InService;
        case SchedulerState::InService:    return SchedulerState::Ready;
        default:                           return SchedulerState::Idle;
    }
}

std::string stateToString(SchedulerState state) {
    switch(state) {
        case SchedulerState::Idle:         return "Idle";
        case SchedulerState::Ready:        return "Ready";
        case SchedulerState::InService:    return "InService";
        default:                           return "Unknown";
    }
}

//----------------------------------------------------------------
// RPC Stub Class
//----------------------------------------------------------------
// This class simulates UDP communication methods used by the Scheduler.
class RPC {
public:
    void openSchedulerSocket() {
        std::cout << "[RPC] Scheduler socket opened." << std::endl;
    }
    
    void closeSchedulerSocket() {
        std::cout << "[RPC] Scheduler socket closed." << std::endl;
    }
    
    // Simulate receiving a reply from the floor subsystem.
    std::string replyFloor() {
        std::cout << "[RPC] Waiting for floor reply..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        // Return a dummy ElevatorRequest string in the expected format.
        return "12:00:00.123 1 UP 5";
    }
    
    // Simulate sending a reply to the elevator.
    void replyElevator(const std::string &data) {
        std::cout << "[RPC] Sending reply to elevator: " << data << std::endl;
    }
    
    // Simulate receiving an acknowledgment from the elevator.
    std::string ackElevator() {
        std::cout << "[RPC] Waiting for elevator ack..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return "12:00:05.456 1 UP 5";
    }
    
    // Simulate sending an acknowledgment back to the floor.
    void ackFloor(const std::string &data) {
        std::cout << "[RPC] Sending ack to floor: " << data << std::endl;
    }
};

//----------------------------------------------------------------
// Scheduler Class
//----------------------------------------------------------------

class Scheduler {
private:
    std::vector<ElevatorRequest> requestsQueue;
    std::vector<ElevatorRequest> completedQueue;
    std::map<int, int> elevatorLocation; // Maps elevator ID to current floor.
    SchedulerState schedulerState;
    RPC rpc;
    
    std::mutex mtx;
    std::condition_variable cv;
    
    // Encodes an ElevatorRequest as a string.
    std::string encodeData(const ElevatorRequest &req) {
        return req.toString();
    }
    
    // Decodes a string into an ElevatorRequest.
    ElevatorRequest decodeData(const std::string &data) {
        std::istringstream iss(data);
        std::string ts, dir;
        int src, dest;
        iss >> ts >> src >> dir >> dest;
        return ElevatorRequest(ts, src, stringToDirection(dir), dest);
    }
    
public:
    Scheduler() : schedulerState(SchedulerState::Idle) {
        std::cout << "[Scheduler] Initialized in state: " << stateToString(schedulerState) << std::endl;
    }
    
    // Adds a new elevator request to the queue (avoiding duplicates).
    void putRequest(const ElevatorRequest &req) {
        std::unique_lock<std::mutex> lock(mtx);
        if (std::find(requestsQueue.begin(), requestsQueue.end(), req) == requestsQueue.end()) {
            requestsQueue.push_back(req);
            std::cout << "[Scheduler] Added request: " << req.toString()
                      << " (Queue size: " << requestsQueue.size() << ")" << std::endl;
            cv.notify_all();
        }
    }
    
    // Removes and returns the next request from the queue (waiting if necessary).
    ElevatorRequest dispatchRequest() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{ return !requestsQueue.empty(); });
        ElevatorRequest req = requestsQueue.front();
        requestsQueue.erase(requestsQueue.begin());
        std::cout << "[Scheduler] Dispatched request: " << req.toString()
                  << " (Queue size: " << requestsQueue.size() << ")" << std::endl;
        cv.notify_all();
        return req;
    }
    
    // Adds a completed request to the completed queue.
    void putCompletedRequest(const ElevatorRequest &req) {
        std::unique_lock<std::mutex> lock(mtx);
        if (std::find(completedQueue.begin(), completedQueue.end(), req) == completedQueue.end()) {
            completedQueue.push_back(req);
            std::cout << "[Scheduler] Added completed request: " << req.toString()
                      << " (Completed size: " << completedQueue.size() << ")" << std::endl;
            cv.notify_all();
        }
    }
    
    // Retrieves a completed request from the completed queue (waiting if necessary).
    ElevatorRequest getCompletedRequest() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{ return !completedQueue.empty(); });
        ElevatorRequest req = completedQueue.front();
        completedQueue.erase(completedQueue.begin());
        cv.notify_all();
        return req;
    }
    
    // Simulates the elevator moving between floors and updates its location.
    int movingTo(int id, int currentFloor, int destinationFloor) {
        while (currentFloor != destinationFloor) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            std::cout << "[Scheduler] Elevator " << id << " moving from floor " 
                      << currentFloor << " to floor " << destinationFloor << std::endl;
            if (currentFloor < destinationFloor)
                currentFloor++;
            else
                currentFloor--;
            registerElevatorLocation(id, currentFloor);
        }
        std::cout << "[Scheduler] Elevator " << id << " arrived at floor " << currentFloor << std::endl;
        return currentFloor;
    }
    
    // Registers the current location of an elevator.
    void registerElevatorLocation(int id, int floorNumber) {
        std::unique_lock<std::mutex> lock(mtx);
        elevatorLocation[id] = floorNumber;
        std::cout << "[Scheduler] Elevator#" << id << " current location: Floor " 
                  << displayElevatorLocation(id) << std::endl;
    }
    
    // Retrieves the current location of an elevator.
    int displayElevatorLocation(int id) {
        std::unique_lock<std::mutex> lock(mtx);
        return elevatorLocation[id];
    }
    
    // The main run loop for the scheduler subsystem.
    void run() {
        rpc.openSchedulerSocket();
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
        while (true) {
            switch (schedulerState) {
                case SchedulerState::Idle:
                    schedulerState = nextState(schedulerState);
                    break;
                    
                case SchedulerState::Ready: {
                    // Simulate receiving a request from a floor.
                    std::string reply = rpc.replyFloor();
                    ElevatorRequest req = decodeData(reply);
                    putRequest(req);
                    
                    // Dispatch a request for an elevator.
                    req = dispatchRequest();
                    std::string data = encodeData(req);
                    rpc.replyElevator(data);
                    
                    schedulerState = nextState(schedulerState);
                    break;
                }
                    
                case SchedulerState::InService: {
                    // Simulate receiving an acknowledgment from an elevator.
                    std::string ack = rpc.ackElevator();
                    ElevatorRequest req = decodeData(ack);
                    putCompletedRequest(req);
                    
                    // Retrieve a completed request and send an acknowledgment to the floor.
                    req = getCompletedRequest();
                    std::string data = encodeData(req);
                    rpc.ackFloor(data);
                    
                    schedulerState = nextState(schedulerState);
                    std::cout << "--------------------------------------" << std::endl;
                    break;
                }
                    
                default:
                    break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        rpc.closeSchedulerSocket();
    }
};

int main() {
    Scheduler scheduler;
    // Run the scheduler; in a full system this might run on its own thread.
    scheduler.run();
    return 0;
}
