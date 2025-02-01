#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <string>

// Represents a request for the elevator system
class ElevatorRequest {
public:
    int time;       // Timestamp of the request
    int floor;      // Floor number
    int buttonType; // Button type (e.g., 0 for down, 1 for up)

    ElevatorRequest(int t = 0, int f = 0, int bt = 0);
    std::string toString() const; // Convert request to string
    bool operator==(const ElevatorRequest& other) const; // Compare requests
};

// Manages elevator requests and coordinates between floors and elevators
class Scheduler {
private:
    std::mutex mtx; // Mutex for thread synchronization
    std::condition_variable cv; // Condition variable for thread signaling
    std::queue<ElevatorRequest> requestsQueue; // Queue for pending requests
    std::queue<ElevatorRequest> completedQueue; // Queue for completed requests
    bool running = true; // Flag to control the scheduler's running state

    // Check if a request already exists in a queue
    bool contains(const std::queue<ElevatorRequest>& q, const ElevatorRequest& req) const;
    // Log messages to the console
    static void log(const std::string& message);

public:
    // Add a new request to the queue
    void putRequest(const ElevatorRequest& req);
    // Retrieve the next request to process (blocks if queue is empty)
    ElevatorRequest dispatchRequest();
    // Mark a request as completed
    void putCompletedRequest(const ElevatorRequest& req);
    // Retrieve a completed request (blocks if queue is empty)
    ElevatorRequest getCompletedRequest();
    // Stop the scheduler
    void stop();
    // Main processing loop (to run in a thread)
    void run();
};

#endif // SCHEDULER_H