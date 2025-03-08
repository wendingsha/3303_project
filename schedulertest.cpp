#include <cassert>
#include <iostream>
#include "scheduler.cpp"  // Include the Scheduler implementation.

int main() {
    Scheduler scheduler;
    
    // Test registering and displaying elevator location.
    scheduler.registerElevatorLocation(100, 1);
    int loc = scheduler.displayElevatorLocation(100);
    assert(loc == 1);
    
    // Test moving the elevator from floor 1 to floor 5.
    int newLoc = scheduler.movingTo(100, loc, 5);
    assert(newLoc == 5);
    assert(scheduler.displayElevatorLocation(100) == 5);
    
    // Test putting and dispatching a request.
    ElevatorRequest req("12:00:00.123", 2, Direction::DOWN, 1);
    scheduler.putRequest(req);
    ElevatorRequest dispatched = scheduler.dispatchRequest();
    assert(dispatched == req);
    
    // Test completed request queue.
    scheduler.putCompletedRequest(req);
    ElevatorRequest completed = scheduler.getCompletedRequest();
    assert(completed == req);
    
    std::cout << "Scheduler tests passed." << std::endl;
    return 0;
}
