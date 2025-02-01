#include "scheduler.hpp"
#include <iostream>
#include <algorithm>
#include <sstream>

// ElevatorRequest implementation
ElevatorRequest::ElevatorRequest(int t, int f, int bt) 
    : time(t), floor(f), buttonType(bt) {}

// Convert request to a string representation
std::string ElevatorRequest::toString() const {
    std::ostringstream oss;
    oss << "Time=" << time << ", Floor=" << floor << ", Button=" << buttonType;
    return oss.str();
}

// Compare two requests for equality
bool ElevatorRequest::operator==(const ElevatorRequest& other) const {
    return time == other.time && 
           floor == other.floor && 
           buttonType == other.buttonType;
}

// Check if a request exists in a queue
bool Scheduler::contains(const std::queue<ElevatorRequest>& q, const ElevatorRequest& req) const {
    auto temp = q;
    while (!temp.empty()) {
        if (temp.front() == req) return true;
        temp.pop();
    }
    return false;
}

// Log messages to the console
void Scheduler::log(const std::string& message) {
    std::cout << "[Scheduler] " << message << std::endl;
}

// Add a new request to the queue
void Scheduler::putRequest(const ElevatorRequest& req) {
    std::lock_guard<std::mutex> lock(mtx);
    
    // Avoid duplicate requests
    if (!contains(requestsQueue, req)) {
        requestsQueue.push(req);
        log("Added " + req.toString() + " | Queue size: " + std::to_string(requestsQueue.size()));
    }
    cv.notify_all(); // Notify waiting threads
}

// Retrieve the next request to process
ElevatorRequest Scheduler::dispatchRequest() {
    std::unique_lock<std::mutex> lock(mtx);
    // Wait until there is a request or the scheduler is stopped
    cv.wait(lock, [this] { 
        return !requestsQueue.empty() || !running; 
    });

    // Return a termination signal if the scheduler is stopped and the queue is empty
    if (!running && requestsQueue.empty()) {
        return {-1, -1, -1};
    }

    // Retrieve and remove the next request
    auto req = requestsQueue.front();
    requestsQueue.pop();
    log("Dispatched " + req.toString() + " | Remaining: " + std::to_string(requestsQueue.size()));
    return req;
}

// Mark a request as completed
void Scheduler::putCompletedRequest(const ElevatorRequest& req) {
    std::lock_guard<std::mutex> lock(mtx);
    
    // Avoid duplicate completed requests
    if (!contains(completedQueue, req)) {
        completedQueue.push(req);
        log("Completed " + req.toString() + " | Processed: " + std::to_string(completedQueue.size()));
    }
    cv.notify_all(); // Notify waiting threads
}

// Retrieve a completed request
ElevatorRequest Scheduler::getCompletedRequest() {
    std::unique_lock<std::mutex> lock(mtx);
    // Wait until there is a completed request or the scheduler is stopped
    cv.wait(lock, [this] { 
        return !completedQueue.empty() || !running; 
    });

    // Return a termination signal if the scheduler is stopped and the queue is empty
    if (!running && completedQueue.empty()) {
        return {-1, -1, -1};
    }

    // Retrieve and remove the next completed request
    auto req = completedQueue.front();
    completedQueue.pop();
    return req;
}

// Stop the scheduler
void Scheduler::stop() {
    std::lock_guard<std::mutex> lock(mtx);
    running = false;
    cv.notify_all(); // Notify all waiting threads
}

// Main processing loop (to run in a thread)
void Scheduler::run() {
    while (running) {
        // Simulate periodic processing
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}