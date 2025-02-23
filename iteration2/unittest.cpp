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
extern std::vector<bool> elevatorBusy; // Two elevators, initial state idle


// Test floor request generation
TEST(FloorTest, RequestGeneration) {
    // Start floor thread
    std::thread floorThread(floorFunction);

    // Simulate a request
    {
        std::lock_guard<std::mutex> lock(mtx);
        ElevatorMessage msg = {0, 3, 5, true, 0}; // From floor 3 to floor 5
        elevatorQueue.push(msg);
    }
    cv.notify_all();

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
        ElevatorMessage msg = {0, 3, 5, true, 0}; // From floor 3 to floor 5
        schedulerQueue.push(msg);
    }
    cv.notify_all(); // Notify scheduler thread

    // Wait for scheduler to assign the request
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Check if elevator queue has the assigned request
    {
        std::lock_guard<std::mutex> lock(mtx);
        EXPECT_FALSE(elevatorQueue.empty()); // Elevator queue should not be empty
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
