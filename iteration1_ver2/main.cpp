#include "message.hpp"
#include "floor.hpp"
#include "scheduler.hpp"
#include "elevator.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>

std::mutex mtx;
std::condition_variable cv;
std::queue<ElevatorMessage> schedulerQueue;
std::queue<ElevatorMessage> elevatorQueue;
std::vector<int> elevatorPositions = {0, 0}; // Two elevators
bool systemActive = true;

int main() {
    std::thread floorThread(floorFunction);
    std::thread schedulerThread(schedulerFunction);
    std::thread elevator1Thread(elevatorFunction, 0);
    std::thread elevator2Thread(elevatorFunction, 1);

    std::this_thread::sleep_for(std::chrono::seconds(15)); // Run for a while
    systemActive = false; // Stop simulation

    floorThread.join();
    schedulerThread.join();
    elevator1Thread.join();
    elevator2Thread.join();

    return 0;
}
