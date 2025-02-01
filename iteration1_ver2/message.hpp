#ifndef MESSAGE_HPP
#define MESSAGE_HPP

struct ElevatorMessage {
    int requestId;      // Unique ID for request tracking
    int floorNumber;    // Floor where request originated
    int destination;    // Requested destination floor
    bool isUp;          // True if the request is an "up" request, false for "down"
    int assignedElevator; // Assigned elevator ID (0 if not yet assigned)
};

#endif
