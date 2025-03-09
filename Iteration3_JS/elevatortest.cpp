// elevator_test.cpp
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

// --- Dummy definitions for test ---
// Assuming the ElevatorMessage and ElevatorState are defined as follows:
enum class ElevatorState { IDLE, DOOR_OPEN, DOOR_CLOSED, MOVING };

struct ElevatorMessage {
    int floorNumber;
    int destination;
    int timestamp;
    int status;
};

// Dummy updateTime function (normally defined in time_manager.hpp)
void updateTime(int newTime) {
    extern int currentTime;
    if (newTime > currentTime) {
        currentTime = newTime;
    }
}

// Globals declared in elevator.cpp
int currentTime = 0;
std::mutex printMutex;
bool systemActive = true;

// Constants defined in elevator.cpp
#define ELEVATOR_PORT_BASE 9100
#define SCHEDULER_IP "127.0.0.1"
#define SCHEDULER_PORT 8100
#define FLOOR_TRAVEL_TIME 1.0
#define MIN_FLOOR 1

// Prototype of the function under test (defined in elevator.cpp)
extern void elevatorFunction(int elevatorId);

//
// Test fixture for elevator tests
//
class ElevatorTest : public ::testing::Test {
protected:
    // UDP socket for receiving messages from the elevator (scheduler simulation)
    int schedulerSock;
    sockaddr_in schedulerAddr;

    void SetUp() override {
        // Create a UDP socket and bind it to the scheduler port (8100)
        schedulerSock = socket(AF_INET, SOCK_DGRAM, 0);
        ASSERT_NE(schedulerSock, -1) << "Failed to create scheduler socket";

        memset(&schedulerAddr, 0, sizeof(schedulerAddr));
        schedulerAddr.sin_family = AF_INET;
        schedulerAddr.sin_addr.s_addr = INADDR_ANY;
        schedulerAddr.sin_port = htons(SCHEDULER_PORT);

        int bindRes = bind(schedulerSock, (struct sockaddr*)&schedulerAddr, sizeof(schedulerAddr));
        ASSERT_NE(bindRes, -1) << "Failed to bind scheduler socket";

        // Set a receive timeout (5 seconds) to avoid blocking indefinitely
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(schedulerSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    }

    void TearDown() override {
        close(schedulerSock);
    }
};

TEST_F(ElevatorTest, ProcessesAssignmentMessage) {
    // Launch the elevator function in a separate thread (using elevatorId 0)
    std::thread elevatorThread(elevatorFunction, 0);
    // Allow some time for the elevator to initialize and bind its socket
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Create a socket to send an assignment message to the elevator
    int senderSock = socket(AF_INET, SOCK_DGRAM, 0);
    ASSERT_NE(senderSock, -1) << "Failed to create sender socket";

    sockaddr_in elevatorAddr;
    memset(&elevatorAddr, 0, sizeof(elevatorAddr));
    elevatorAddr.sin_family = AF_INET;
    elevatorAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    elevatorAddr.sin_port = htons(ELEVATOR_PORT_BASE + 0); // For elevatorId 0

    // Prepare an assignment message:
    // For this test, we set pickup floor to MIN_FLOOR (1) and destination to 5.
    ElevatorMessage msg;
    msg.floorNumber = MIN_FLOOR; // pickup floor is the same as the current floor initially
    msg.destination = 5;
    msg.timestamp = 0;
    msg.status = 0;

    int sendRes = sendto(senderSock, &msg, sizeof(msg), 0,
                         (sockaddr*)&elevatorAddr, sizeof(elevatorAddr));
    ASSERT_NE(sendRes, -1) << "Failed to send assignment message";

    // The elevator should process the message and then send a response to the scheduler.
    // First, wait for the response ElevatorMessage (with updated status and timestamp).
    ElevatorMessage responseMsg;
    sockaddr_in fromAddr;
    socklen_t fromLen = sizeof(fromAddr);
    int recvRes = recvfrom(schedulerSock, &responseMsg, sizeof(responseMsg), 0,
                           (sockaddr*)&fromAddr, &fromLen);
    ASSERT_NE(recvRes, -1) << "Did not receive response message from elevator";

    // Verify that the response message has been updated:
    // Expect status to be 1, destination same as assigned, and timestamp increased.
    EXPECT_EQ(responseMsg.status, 1);
    EXPECT_EQ(responseMsg.destination, 5);
    EXPECT_EQ(responseMsg.floorNumber, MIN_FLOOR);
    // Since travel time = |destination - currentFloor| * FLOOR_TRAVEL_TIME
    // Here: |5 - 1| * 1 = 4 seconds (plus any delays), so currentTime should be >= 4.
    EXPECT_GE(responseMsg.timestamp, 4);

    // Next, the elevator sends an idle status (sent as a bool).
    bool idleStatus;
    recvRes = recvfrom(schedulerSock, &idleStatus, sizeof(idleStatus), 0,
                       (sockaddr*)&fromAddr, &fromLen);
    ASSERT_NE(recvRes, -1) << "Did not receive idle status from elevator";
    EXPECT_TRUE(idleStatus);

    // Clean up: Signal the elevator function to stop.
    systemActive = false;
    // Send a dummy message to unblock the elevator's recvfrom call.
    ElevatorMessage dummyMsg = {0, 0, 0, 0};
    sendto(senderSock, &dummyMsg, sizeof(dummyMsg), 0,
           (sockaddr*)&elevatorAddr, sizeof(elevatorAddr));

    // Wait for the elevator thread to finish.
    elevatorThread.join();
    close(senderSock);
}
