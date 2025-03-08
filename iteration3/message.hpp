#ifndef MESSAGE_HPP
#define MESSAGE_HPP

enum class ElevatorState { IDLE, MOVING, STOPPING, DOOR_OPEN, DOOR_CLOSED };

struct ElevatorMessage {
    int requestId;
    int floorNumber;
    int destination;
    bool goingUp;
    int assignedElevator;
};

#endif // MESSAGE_HPP
