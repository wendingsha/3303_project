// floor_test.cpp
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <set>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

// Dummy ElevatorMessage definition matching the one used in floor.cpp.
// In your project, this would come from "message.hpp".
struct ElevatorMessage {
    int floorNumber;
    int destination;
    bool direction;     // True if destination > floorNumber
    int someField;      // Expected to be -1 based on floor.cpp
    int status;         // Expected to be 0 based on floor.cpp
    int timestamp;      // Set in floorFunction

    ElevatorMessage() {}
    ElevatorMessage(int f, int d, bool dir, int sf, int st)
        : floorNumber(f), destination(d), direction(dir),
          someField(sf), status(st), timestamp(0) {}
};

// Extern declarations (if needed) for the global mutex and the function under test.
extern std::mutex printMutex; // used in floor.cpp for synchronized printing

// floorFunction is defined in floor.cpp.
extern void floorFunction();

// Constants defined in floor.cpp
#define SCHEDULER_IP "127.0.0.1"
#define SCHEDULER_PORT 8100

// Test fixture for floor tests.
class FloorTest : public ::testing::Test {
protected:
    int schedulerSock;
    sockaddr_in schedulerAddr;

    void SetUp() override {
        // Create a UDP socket to simulate the scheduler
        schedulerSock = socket(AF_INET, SOCK_DGRAM, 0);
        ASSERT_NE(schedulerSock, -1) << "Failed to create scheduler socket";

        memset(&schedulerAddr, 0, sizeof(schedulerAddr));
        schedulerAddr.sin_family = AF_INET;
        schedulerAddr.sin_addr.s_addr = INADDR_ANY;
        schedulerAddr.sin_port = htons(SCHEDULER_PORT);

        int bindRes = bind(schedulerSock, (struct sockaddr*)&schedulerAddr, sizeof(schedulerAddr));
        ASSERT_NE(bindRes, -1) << "Failed to bind scheduler socket";

        // Set a receive timeout to prevent blocking indefinitely.
        struct timeval tv;
        tv.tv_sec = 5;  // 5 seconds timeout per recvfrom call
        tv.tv_usec = 0;
        int ret = setsockopt(schedulerSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        ASSERT_EQ(ret, 0) << "Failed to set socket timeout";
    }

    void TearDown() override {
        close(schedulerSock);
    }
};

TEST_F(FloorTest, SendsFloorRequests) {
    // Expected list of requests based on floor.cpp:
    // requestList = { {1,5}, {3,8}, {6,2}, {9,4}, {2,7}, {8,1}, {4,10} }
    // And timer is incremented by 4 after each request starting at 0.
    struct ExpectedMessage {
        int floorNumber;
        int destination;
        bool direction;
        int timestamp;
    };
    std::vector<ExpectedMessage> expected = {
        {1, 5, true, 0},
        {3, 8, true, 4},
        {6, 2, false, 8},
        {9, 4, false, 12},
        {2, 7, true, 16},
        {8, 1, false, 20},
        {4, 10, true, 24}
    };

    // Launch floorFunction in a separate thread.
    // Note: floorFunction sleeps between sending messages.
    std::thread floorThread(floorFunction);

    // Give floorFunction a moment to start up (it sleeps for 500ms initially)
    std::this_thread::sleep_for(std::chrono::milliseconds(600));

    std::vector<ElevatorMessage> receivedMessages;
    sockaddr_in fromAddr;
    socklen_t fromLen = sizeof(fromAddr);
    const size_t expectedCount = expected.size();

    // Loop to receive each expected message.
    for (size_t i = 0; i < expectedCount; ++i) {
        ElevatorMessage msg;
        int recvRes = recvfrom(schedulerSock, &msg, sizeof(msg), 0, (struct sockaddr*)&fromAddr, &fromLen);
        ASSERT_NE(recvRes, -1) << "Did not receive expected message #" << i+1;
        receivedMessages.push_back(msg);
    }

    // Verify each received message matches expectations.
    for (size_t i = 0; i < expectedCount; ++i) {
        EXPECT_EQ(receivedMessages[i].floorNumber, expected[i].floorNumber)
            << "Mismatch in floorNumber for message #" << i+1;
        EXPECT_EQ(receivedMessages[i].destination, expected[i].destination)
            << "Mismatch in destination for message #" << i+1;
        EXPECT_EQ(receivedMessages[i].direction, expected[i].direction)
            << "Mismatch in direction for message #" << i+1;
        EXPECT_EQ(receivedMessages[i].timestamp, expected[i].timestamp)
            << "Mismatch in timestamp for message #" << i+1;
        EXPECT_EQ(receivedMessages[i].someField, -1)
            << "Mismatch in someField for message #" << i+1;
        EXPECT_EQ(receivedMessages[i].status, 0)
            << "Mismatch in status for message #" << i+1;
    }

    // Join the floorFunction thread.
    floorThread.join();
}
