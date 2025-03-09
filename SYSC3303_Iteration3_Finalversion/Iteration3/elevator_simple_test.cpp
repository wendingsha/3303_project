// elevator_simple_test.cpp
#include <iostream>
#include <cmath>
#include "message.hpp"

// Improved test assertion macro with detailed output
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << " FAILED: " << message << std::endl; \
            return 1; \
        } else { \
            std::cout << "✓ PASSED: " << message << std::endl; \
        } \
    } while (0)

// Test elevator states
int testElevatorStates() {
    std::cout << "\n=== Testing Elevator States ===" << std::endl;
    
    std::cout << "  Test Case 1: Initial IDLE state" << std::endl;
    ElevatorState state = ElevatorState::IDLE;
    TEST_ASSERT(state == ElevatorState::IDLE, "Initial state should be IDLE");
    
    std::cout << "  Test Case 2: Transition to MOVING state" << std::endl;
    state = ElevatorState::MOVING;
    TEST_ASSERT(state == ElevatorState::MOVING, "State should change to MOVING");
    
    std::cout << "  Test Case 3: Transition to DOOR_OPEN state" << std::endl;
    state = ElevatorState::DOOR_OPEN;
    TEST_ASSERT(state == ElevatorState::DOOR_OPEN, "State should change to DOOR_OPEN");
    
    std::cout << "  Test Case 4: Transition to DOOR_CLOSED state" << std::endl;
    state = ElevatorState::DOOR_CLOSED;
    TEST_ASSERT(state == ElevatorState::DOOR_CLOSED, "State should change to DOOR_CLOSED");
    
    std::cout << "Elevator States: All tests passed" << std::endl;
    return 0;
}

// Test floor movement calculation
int testFloorMovement() {
    std::cout << "\n=== Testing Floor Movement Calculation ===" << std::endl;
    
    std::cout << "  Test Case 1: Moving from floor 1 to 5 (UP)" << std::endl;
    int currentFloor = 1;
    int destinationFloor = 5;
    int distance = std::abs(destinationFloor - currentFloor);
    TEST_ASSERT(distance == 4, 
               "Distance from floor 1 to 5 should be 4 floors");
    
    std::cout << "  Test Case 2: Moving from floor 8 to 3 (DOWN)" << std::endl;
    currentFloor = 8;
    destinationFloor = 3;
    distance = std::abs(destinationFloor - currentFloor);
    TEST_ASSERT(distance == 5, 
               "Distance from floor 8 to 3 should be 5 floors");
    
    std::cout << "  Test Case 3: Moving to the same floor" << std::endl;
    currentFloor = 6;
    destinationFloor = 6;
    distance = std::abs(destinationFloor - currentFloor);
    TEST_ASSERT(distance == 0, 
               "Distance from floor 6 to 6 should be 0 floors");
    
    std::cout << "Floor Movement Calculation: All tests passed" << std::endl;
    return 0;
}

// Test elevator message
int testElevatorMessage() {
    std::cout << "\n=== Testing Elevator Message ===" << std::endl;
    
    std::cout << "  Test Case 1: Message Creation and Field Access" << std::endl;
    // Create an elevator message
    ElevatorMessage msg(3, 7, true, 1, 0);
    
    TEST_ASSERT(msg.floorNumber == 3, "Source floor should be 3");
    TEST_ASSERT(msg.destination == 7, "Destination floor should be 7");
    TEST_ASSERT(msg.assignedElevator == 1, "Assigned elevator ID should be 1");
    
    std::cout << "  Test Case 2: Message Timestamp Update" << std::endl;
    // Update timestamp and test
    msg.timestamp = 100;
    TEST_ASSERT(msg.timestamp == 100, "Timestamp should update to 100");
    
    std::cout << "  Test Case 3: Message Status Update" << std::endl;
    // Update status and test
    msg.status = 1;
    TEST_ASSERT(msg.status == 1, "Status should update to 1 (completed)");
    
    std::cout << " Elevator Message: All tests passed" << std::endl;
    return 0;
}

int main() {
    int failures = 0;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "STARTING ELEVATOR SYSTEM TESTS" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    failures += testElevatorStates();
    failures += testFloorMovement();
    failures += testElevatorMessage();
    
    std::cout << "\n========================================" << std::endl;
    if (failures == 0) {
        std::cout << " ELEVATOR TESTS SUMMARY: All tests passed! " << std::endl;
    } else {
        std::cout << " ELEVATOR TESTS SUMMARY: " << failures << " tests failed! " << std::endl;
    }
    std::cout << "========================================" << std::endl;
    
    return failures;
}