#include <cassert>
#include <iostream>
#include "floor.cpp"  // Include the Floor (and Parser, FloorComponents) implementation.
#include <string>

int main() {
    // Test that a Floor object returns the correct floor number.
    Floor floorSubsystem(3, "test_input.txt");
    assert(floorSubsystem.getFloorNumber() == 3);
    
    // Additionally, test the Parser functionality.
    Parser parser("dummy.txt");
    // Test that fillTimestampZero pads the timestamp properly.
    std::string filled = parser.fillTimestampZero("07:01:15.5 2 UP 6");
    assert(filled == "07:01:15.500 2 UP 6");
    
    // Test parsing a valid request string.
    ElevatorRequest req = parser.textParser("07:01:15.000 2 UP 6");
    assert(req.sourceFloor == 2);
    assert(req.destinationFloor == 6);
    assert(req.direction == Direction::UP);
    
    std::cout << "Floor tests passed." << std::endl;
    return 0;
}
