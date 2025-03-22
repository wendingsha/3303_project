#ifndef MESSAGE_HPP
#define MESSAGE_HPP

// Enumerations for elevator and scheduler states (if needed)
enum ElevatorState {
    IDLE,
    DOOR_OPEN,
    DOOR_CLOSED,
    MOVING
};

enum SchedulerState {
    IDLE_SCHEDULER,
    ASSIGNING,
    PROCESSING
};

// Message structure for communication among floor, scheduler, and elevator.
struct ElevatorMessage {
    int floorNumber;         // Pickup (or current) floor
    int destination;         // Destination floor
    bool directionUp;        // true for UP request; false for DOWN
    int assignedElevator;    // Elevator id assigned (-1 if not yet assigned)
    int status;              // 1 for success, negative for faults
    int msgType;             // 0: new request/assignment, 1: normal completion, 2: fault, 3: intermediate update
    int faultCode;           // 0: no fault, 1: door fault, 2: elevator stuck fault
    int timestamp;           // Simulated time when the message is sent

    ElevatorMessage() 
        : floorNumber(0), destination(0), directionUp(true), assignedElevator(-1),
          status(0), msgType(0), faultCode(0), timestamp(0) {}

    ElevatorMessage(int floor, int dest, bool up, int assigned, int ts) 
        : floorNumber(floor), destination(dest), directionUp(up), assignedElevator(assigned),
          status(0), msgType(0), faultCode(0), timestamp(ts) {}
};

#endif // MESSAGE_HPP
