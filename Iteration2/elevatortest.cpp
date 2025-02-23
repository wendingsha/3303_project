#include <gtest/gtest.h>
#include "elevator.hpp"
#include "floor.hpp"
#include "scheduler.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>

// Mock global variables
std::mutex mtx;
std::condition_variable cv;
std::queue<ElevatorMessage> schedulerQueue;
std::queue<ElevatorMessage> elevatorQueue;
std::vector<int> elevatorPositions = {0, 0}; // Two elevators, initial position 0
bool systemActive = true;
std::vector<bool> elevatorBusy = {false, false}; // Two elevators, initial state idle

// Test elevator state transitions
TEST(ElevatorTest, StateTransitions) {
    // Start elevator thread
    std::thread elevatorThread(elevatorFunction, 0);

    // Simulate a request
    {
        std::lock_guard<std::mutex> lock(mtx);
        ElevatorMessage msg = {3, 5}; // From floor 3 to floor 5
        elevatorQueue.push(msg);
    }
    cv.notify_all(); // Notify elevator thread

    // Wait for elevator to complete the task
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    // Check elevator's final state and position
    {
        std::lock_guard<std::mutex> lock(mtx);
        EXPECT_EQ(elevatorPositions[0], 5); // Elevator final position should be floor 5
        EXPECT_FALSE(elevatorBusy[0]);      // Elevator should be idle
    }

    // Stop the system
    systemActive = false;
    cv.notify_all(); // Ensure elevator thread exits
    elevatorThread.join();
}

// Test floor request generation
TEST(FloorTest, RequestGeneration) {
    // Start floor thread
    std::thread floorThread(floorFunction);

    // Wait for floor thread to generate requests
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Check if scheduler queue has requests
    {
        std::lock_guard<std::mutex> lock(mtx);
        EXPECT_FALSE(schedulerQueue.empty()); // Scheduler queue should not be empty
    }

    // Stop the system
    systemActive = false;
    cv.notify_all(); // Ensure floor thread exits
    floorThread.join();
}

// Test scheduler request assignment
TEST(SchedulerTest, RequestAssignment) {
    // Start scheduler thread
    std::thread schedulerThread(schedulerFunction);

    // Simulate a request
    {
        std::lock_guard<std::mutex> lock(mtx);
        ElevatorMessage msg = {3, 5}; // From floor 3 to floor 5
        schedulerQueue.push(msg);
    }
    cv.notify_all(); // Notify scheduler thread

    // Wait for scheduler to assign the request
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Check if elevator queue has the assigned request
    {
        std::lock_guard<std::mutex> lock(mtx);
        EXPECT_FALSE(elevatorQueue.empty()); // Elevator queue should not be empty
        if (!elevatorQueue.empty()) {
            ElevatorMessage assignedMsg = elevatorQueue.front();
            EXPECT_EQ(assignedMsg.floorNumber, 3); // Check pickup floor
            EXPECT_EQ(assignedMsg.destination, 5); // Check destination floor
        }
    }

    // Stop the system
    systemActive = false;
    cv.notify_all(); // Ensure scheduler thread exits
    schedulerThread.join();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}