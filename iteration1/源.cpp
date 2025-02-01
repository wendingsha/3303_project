#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <sstream>
#include <string>

struct Event {
    int time;
    int floorOrElevator;
    std::string button;
};

std::queue<Event> floorToScheduler;
std::queue<Event> schedulerToElevator;
std::queue<Event> elevatorToScheduler;
std::queue<Event> schedulerToFloor;

std::mutex mtx;
std::condition_variable cv;

// Floor Subsystem
void floor_system(const std::string& filename) {
    std::ifstream infile(filename);
    std::string line;

    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        Event event;
        if (!(iss >> event.time >> event.floorOrElevator >> event.button)) break;

        {
            std::lock_guard<std::mutex> lock(mtx);
            floorToScheduler.push(event);
            std::cout << "Floor sent event: Time " << event.time << ", Floor " << event.floorOrElevator << ", Button " << event.button << std::endl;
        }
        cv.notify_all();

        // Wait for response
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return !schedulerToFloor.empty(); });

        Event response = schedulerToFloor.front();
        schedulerToFloor.pop();
        std::cout << "Floor received response for Floor " << response.floorOrElevator << " with Button " << response.button << std::endl;
    }
}

// Scheduler Subsystem
void scheduler_system() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return !floorToScheduler.empty() || !elevatorToScheduler.empty(); });

        if (!floorToScheduler.empty()) {
            Event event = floorToScheduler.front();
            floorToScheduler.pop();
            schedulerToElevator.push(event);
            std::cout << "Scheduler forwarded event to Elevator." << std::endl;
            cv.notify_all();
        }

        if (!elevatorToScheduler.empty()) {
            Event event = elevatorToScheduler.front();
            elevatorToScheduler.pop();
            schedulerToFloor.push(event);
            std::cout << "Scheduler forwarded response to Floor." << std::endl;
            cv.notify_all();
        }
    }
}

// Elevator Subsystem
void elevator_system() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return !schedulerToElevator.empty(); });

        Event event = schedulerToElevator.front();
        schedulerToElevator.pop();

        std::cout << "Elevator processing request for Floor " << event.floorOrElevator << " with Button " << event.button << std::endl;

        elevatorToScheduler.push(event);
        cv.notify_all();
    }
}

int main() {
    std::thread floorThread(floor_system, "input.txt");
    std::thread schedulerThread(scheduler_system);
    std::thread elevatorThread(elevator_system);

    floorThread.join();
    schedulerThread.detach(); // Detach scheduler and elevator as they run indefinitely
    elevatorThread.detach();

    return 0;
}
