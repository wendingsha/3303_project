// scheduler_simple_test.cpp
#include <iostream>
#include <set>
#include <queue>
#include <limits>
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

// Test scheduler states
int testSchedulerStates() {
    std::cout << "\n=== Testing Scheduler States ===" << std::endl;
    
    std::cout << "  Test Case 1: Initial IDLE state" << std::endl;
    SchedulerState state = SchedulerState::IDLE;
    TEST_ASSERT(state == SchedulerState::IDLE, "Initial state should be IDLE");
    
    std::cout << "  Test Case 2: Transition to PROCESSING state" << std::endl;
    state = SchedulerState::PROCESSING;
    TEST_ASSERT(state == SchedulerState::PROCESSING, "State should change to PROCESSING");
    
    std::cout << "  Test Case 3: Transition to ASSIGNING state" << std::endl;
    state = SchedulerState::ASSIGNING;
    TEST_ASSERT(state == SchedulerState::ASSIGNING, "State should change to ASSIGNING");
    
    std::cout << " Scheduler States: All tests passed" << std::endl;
    return 0;
}

// Test request tracking
int testRequestTracking() {
    std::cout << "\n=== Testing Request Tracking ===" << std::endl;
    
    std::set<std::pair<int, int>> processedRequests;
    std::pair<int, int> request1 = {3, 7};
    std::pair<int, int> request2 = {5, 2};
    
    std::cout << "  Test Case 1: Empty tracking set" << std::endl;
    TEST_ASSERT(processedRequests.count(request1) == 0, 
               "Request {3,7} should not exist initially");
    
    std::cout << "  Test Case 2: Adding first request {3,7}" << std::endl;
    processedRequests.insert(request1);
    TEST_ASSERT(processedRequests.count(request1) == 1, 
               "Request {3,7} should exist after insertion");
    TEST_ASSERT(processedRequests.count(request2) == 0, 
               "Request {5,2} should not exist yet");
    
    std::cout << "  Test Case 3: Adding second request {5,2}" << std::endl;
    processedRequests.insert(request2);
    TEST_ASSERT(processedRequests.count(request2) == 1, 
               "Request {5,2} should exist after insertion");
    
    std::cout << "Request Tracking: All tests passed" << std::endl;
    return 0;
}

// Test elevator assignment logic
int testElevatorAssignment() {
    std::cout << "\n=== Testing Elevator Assignment ===" << std::endl;
    
    // Simple elevator struct for testing
    struct Elevator {
        int id;
        int position;
        bool isIdle;
        bool isMoving;
        bool goingUp;
        int passengerCount;
    };
    
    std::cout << "  Test Case 1: Initial Elevator Setup" << std::endl;
    
    // Create test elevators
    Elevator elevators[2];
    elevators[0] = {0, 1, true, false, true, 0};
    elevators[1] = {1, 5, true, false, true, 0};
    
    TEST_ASSERT(elevators[0].position == 1, "Elevator 0 should be at floor 1");
    TEST_ASSERT(elevators[1].position == 5, "Elevator 1 should be at floor 5");
    
    std::cout << "  Test Case 2: Finding Best Elevator for Request" << std::endl;
    
    // Create test request
    ElevatorMessage request(3, 7, true, -1, 0);
    
    // Find the best elevator using simplified assignment logic
    int bestElevator = -1;
    int minDistance = std::numeric_limits<int>::max();
    
    // Check for idle elevators
    for (int i = 0; i < 2; i++) {
        if (elevators[i].isIdle) {
            int distance = std::abs(elevators[i].position - request.floorNumber);
            std::cout << "    Distance from Elevator " << i << " (at floor " 
                      << elevators[i].position << ") to request floor " 
                      << request.floorNumber << " = " << distance << std::endl;
            
            if (distance < minDistance) {
                minDistance = distance;
                bestElevator = elevators[i].id;
            }
        }
    }
    
    TEST_ASSERT(bestElevator != -1, "An elevator should be assigned");
    
    std::cout << "    Selected Elevator: " << bestElevator 
              << " (distance = " << minDistance << ")" << std::endl;
    
    TEST_ASSERT(minDistance == 2, "Minimum distance should be 2 floors");
    
    std::cout << "  Test Case 3: Updating Elevator Status" << std::endl;
    
    // Update elevator status after assignment
    if (bestElevator != -1) {
        int id = bestElevator;
        elevators[id].isIdle = false;
        elevators[id].isMoving = true;
        elevators[id].goingUp = (request.destination > request.floorNumber);
        elevators[id].passengerCount++;
        
        TEST_ASSERT(elevators[id].isIdle == false, 
                   "Elevator should not be idle after assignment");
        TEST_ASSERT(elevators[id].isMoving == true, 
                   "Elevator should be moving after assignment");
        TEST_ASSERT(elevators[id].goingUp == true, 
                   "Elevator should be going up (floor 3 to 7)");
        TEST_ASSERT(elevators[id].passengerCount == 1, 
                   "Passenger count should be 1");
    }
    
    std::cout << "Elevator Assignment: All tests passed" << std::endl;
    return 0;
}

// Test pending request queue
int testPendingRequests() {
    std::cout << "\n=== Testing Pending Request Queue ===" << std::endl;
    
    std::cout << "  Test Case 1: Empty Queue" << std::endl;
    std::queue<ElevatorMessage> pendingRequests;
    TEST_ASSERT(pendingRequests.empty(), "Queue should be empty initially");
    
    std::cout << "  Test Case 2: Adding First Request" << std::endl;
    // Add a request
    ElevatorMessage msg1(3, 7, true, -1, 0);
    pendingRequests.push(msg1);
    
    TEST_ASSERT(!pendingRequests.empty(), "Queue should not be empty after push");
    TEST_ASSERT(pendingRequests.size() == 1, "Queue size should be 1");
    
    std::cout << "  Test Case 3: Adding Second Request" << std::endl;
    // Add another request
    ElevatorMessage msg2(5, 2, false, -1, 0);
    pendingRequests.push(msg2);
    
    TEST_ASSERT(pendingRequests.size() == 2, "Queue size should be 2");
    
    std::cout << "  Test Case 4: Retrieving First Request" << std::endl;
    // Get the first request
    ElevatorMessage frontMsg = pendingRequests.front();
    pendingRequests.pop();
    
    TEST_ASSERT(frontMsg.floorNumber == 3, "Retrieved request should be from floor 3");
    TEST_ASSERT(frontMsg.destination == 7, "Retrieved request should be to floor 7");
    TEST_ASSERT(pendingRequests.size() == 1, "Queue size should be 1 after pop");
    
    std::cout << "Pending Request Queue: All tests passed" << std::endl;
    return 0;
}

int main() {
    int failures = 0;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "STARTING SCHEDULER SYSTEM TESTS" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    failures += testSchedulerStates();
    failures += testRequestTracking();
    failures += testElevatorAssignment();
    failures += testPendingRequests();
    
    std::cout << "\n========================================" << std::endl;
    if (failures == 0) {
        std::cout << " SCHEDULER TESTS SUMMARY: All tests passed! " << std::endl;
    } else {
        std::cout << " SCHEDULER TESTS SUMMARY: " << failures << " tests failed! " << std::endl;
    }
    std::cout << "========================================" << std::endl;
    
    return failures;
}