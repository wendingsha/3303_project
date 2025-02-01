#include "scheduler.hpp"
#include <thread>
#include <chrono>

int main() {
    Scheduler scheduler;

    // Start the scheduler thread
    std::thread schedulerThread([&] {
        scheduler.run();
    });

    // Simulate the floor subsystem
    std::thread floorThread([&] {
        // Send three requests
        scheduler.putRequest({100, 3, 1}); // Time=100, Floor=3, Button=1 (Up)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        scheduler.putRequest({200, 5, 0}); // Time=200, Floor=5, Button=0 (Down)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        scheduler.putRequest({300, 2, 1}); // Time=300, Floor=2, Button=1 (Up)
    });

    // Simulate the elevator subsystem
    std::thread elevatorThread([&] {
        while (true) {
            // Retrieve the next request
            auto req = scheduler.dispatchRequest();
            if (req.time == -1) break; // Termination signal
            
            // Simulate processing the request
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            
            // Mark the request as completed
            scheduler.putCompletedRequest(req);
        }
    });

    // Let the system run for 5 seconds
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // Stop the scheduler
    scheduler.stop();

    // Wait for all threads to finish
    floorThread.join();
    elevatorThread.join();
    schedulerThread.join();

    return 0;
}