#include <cassert>
#include <iostream>
#include "elevator.cpp"  // Include the Elevator implementation.

int main() {
    // Create an Elevator with id 1 and total floors 10.
    Elevator elevator(1, 10);
    
    // Test that the elevator's id is set correctly.
    assert(elevator.id == 1);
    
    // Test that the initial state is Idle.
    assert(elevator.state == ElevatorState::Idle);
    
    // Note: We avoid calling run() since it loops indefinitely.
    std::cout << "Elevator tests passed." << std::endl;
    return 0;
}
