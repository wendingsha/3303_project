#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <mutex>
#include <iostream>


#define MIN_FLOOR 1 
#define MAX_ELEVATORS 2  
extern std::mutex printMutex; 


enum class ElevatorState {
    IDLE,
    MOVING,
    STOPPING,
    DOOR_OPEN,
    DOOR_CLOSED
};


enum class SchedulerState {
    IDLE,
    PROCESSING,
    ASSIGNING
};

// Structure for Elevator Messages sent between Floor, Scheduler, and Elevator
struct ElevatorMessage {
    int floorNumber;        
    int destination;        
    bool directionUp;       
    int assignedElevator;   
    int status;             
    int timestamp;          

    // Default Constructor
    ElevatorMessage() 
        : floorNumber(0), destination(0), directionUp(true), assignedElevator(-1), status(0), timestamp(0) {}

    // No timestamp, defaults to 0
    ElevatorMessage(int floor, int dest, bool up, int elevator, int stat)
        : floorNumber(floor), 
        destination(dest >= MIN_FLOOR ? dest : MIN_FLOOR),  // ✅ Prevent invalid destinations
        directionUp(up), 
        assignedElevator(elevator), 
        status(stat), 
        timestamp(0) {}

    // With timestamp
    ElevatorMessage(int floor, int dest, bool up, int elevator, int stat, int time)
        : floorNumber(floor), destination(dest), directionUp(up), assignedElevator(elevator), status(stat), timestamp(time) {}


    void print() const {
        std::lock_guard<std::mutex> lock(printMutex);
        std::cout << "Request from Floor " << floorNumber << " to Floor " << destination
                  << " (Direction: " << (directionUp ? "Up" : "Down") << ", Assigned Elevator: " 
                  << assignedElevator << ", Status: " << status << ", Timestamp: " << timestamp << ")\n";
    }
};

#endif // MESSAGE_HPP
