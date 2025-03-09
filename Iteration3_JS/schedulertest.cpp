// scheduler_test.cpp
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

// Dummy definitions matching those used in scheduler.cpp.
// Adjust these if your actual ElevatorMessage and updateTime differ.
struct ElevatorMessage {
    int floorNumber;
    int destination;
    bool directionUp;      // true if destination > floorNumber
    int assignedElevator;  // to be set by the scheduler
    int timestamp;
    int status;
};

// Dummy updateTime function (normally provided in time_manager.hpp)
void updateTime(int newTime) {
    // For testing purposes, do nothing.
}

// Global flag declared as extern in scheduler.cpp.
bool systemActive = true;

// Constants (should match those in scheduler.cpp)
#define SCHEDULER_PORT 8100
#define ELEVATOR_PORT_BASE 9100
#define MIN_FLOOR 1

// The schedulerFunction is defined in scheduler.cpp.
extern void schedulerFunction();

//------------------------------------------------------------------------------
// Test fixture for scheduler tests.
//------------------------------------------------------------------------------
class SchedulerTest : public ::testing::Test {
protected:
    int elevatorSock;            // Socket to simulate an elevator endpoint.
    sockaddr_in elevatorAddr;    // Address bound to the elevator's UDP port.

    void SetUp() override {
        // Create a UDP socket and bind it to the elevator's port (simulate Elevator 0).
        elevatorSock = socket(AF_INET, SOCK_DGRAM, 0);
        ASSERT_NE(elevatorSock, -1) << "Failed to create elevator socket";

        memset(&elevatorAddr, 0, sizeof(elevatorAddr));
        elevatorAddr.sin_family = AF_INET;
        elevatorAddr.sin_addr.s_addr = INADDR_ANY;
        elevatorAddr.sin_port = htons(ELEVATOR_PORT_BASE + 0); // Elevator with ID 0

        int bindRes = bind(elevatorSock, (struct sockaddr*)&elevatorAddr, sizeof(elevatorAddr));
        ASSERT_NE(bindRes, -1) << "Failed to bind elevator socket";

        // Set a receive timeout so the test does not block indefinitely.
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        int ret = setsockopt(elevatorSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        ASSERT_EQ(ret, 0) << "Failed to set elevator socket timeout";
    }

    void TearDown() override {
        close(elevatorSock);
    }
};

//------------------------------------------------------------------------------
// This test sends a valid ElevatorMessage (from floor) to the scheduler.
// The scheduler should assign an elevator and send the message to the corresponding elevator port.
//------------------------------------------------------------------------------
TEST_F(SchedulerTest, AssignsElevatorToValidRequest) {
    // Start the scheduler in a separate thread.
    std::thread schedulerThread(schedulerFunction);
    // Allow time for the scheduler to start up and bind to port 8100.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Create a UDP socket to simulate a floor sending a request.
    int floorSock = socket(AF_INET, SOCK_DGRAM, 0);
    ASSERT_NE(floorSock, -1) << "Failed to create floor socket";

    sockaddr_in schedulerAddr;
    memset(&schedulerAddr, 0, sizeof(schedulerAddr));
    schedulerAddr.sin_family = AF_INET;
    schedulerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    schedulerAddr.sin_port = htons(SCHEDULER_PORT);

    // Prepare a valid request:
    // For this test, the floorNumber is MIN_FLOOR (1) so an idle elevator at floor 1 should be chosen.
    ElevatorMessage msg;
    msg.floorNumber = MIN_FLOOR;  // 1
    msg.destination = 5;          // Valid destination (5 > 1)
    msg.directionUp = true;       // true since 5 > 1
    msg.timestamp = 100;          // Arbitrary timestamp
    msg.assignedElevator = -1;    // Initial value
    msg.status = 0;

    int sendRes = sendto(floorSock, &msg, sizeof(msg), 0,
                         (sockaddr*)&schedulerAddr, sizeof(schedulerAddr));
    ASSERT_NE(sendRes, -1) << "Failed to send test request to scheduler";

    // The scheduler should process the request and assign it to an elevator.
    // Since both elevators start idle at MIN_FLOOR, the scheduler should assign elevator 0.
    ElevatorMessage assignedMsg;
    sockaddr_in fromAddr;
    socklen_t fromLen = sizeof(fromAddr);
    int recvRes = recvfrom(elevatorSock, &assignedMsg, sizeof(assignedMsg), 0,
                           (sockaddr*)&fromAddr, &fromLen);
    ASSERT_NE(recvRes, -1) << "Did not receive assigned message on elevator socket";

    // Verify that the scheduler correctly set the assigned elevator.
    EXPECT_EQ(assignedMsg.floorNumber, msg.floorNumber);
    EXPECT_EQ(assignedMsg.destination, msg.destination);
    EXPECT_EQ(assignedMsg.directionUp, msg.directionUp);
    EXPECT_EQ(assignedMsg.assignedElevator, 0) << "Expected elevator 0 to be assigned";
    EXPECT_EQ(assignedMsg.timestamp, msg.timestamp);

    close(floorSock);

    // Stop the scheduler's loop.
    systemActive = false;
    // To unblock the scheduler's recvfrom, send a dummy message.
    int dummySock = socket(AF_INET, SOCK_DGRAM, 0);
    ASSERT_NE(dummySock, -1);
    ElevatorMessage dummyMsg = {0, 0, false, -1, 0, 0};
    sendto(dummySock, &dummyMsg, sizeof(dummyMsg), 0,
           (sockaddr*)&schedulerAddr, sizeof(schedulerAddr));
    close(dummySock);

    schedulerThread.join();
}
