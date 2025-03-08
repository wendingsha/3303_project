#include <cassert>
#include <iostream>
#include <chrono>
#include <string>
#include <sstream>
#include <ctime>

// Include the DTO definitions.
// For demonstration, we include "dto.cpp" directly.
// In a real project, you would include a header (e.g. "dto.h").
#include "dto.cpp"

int main() {
    // Construct an ElevatorRequest using a timestamp string.
    std::string ts_str = "2023-03-11 10:15:30.123";
    ElevatorRequest req(ts_str, 1, Direction::UP, 5);
    
    // Test toString() returns a string that contains expected substrings.
    std::string s = req.toString();
    std::cout << "ElevatorRequest.toString(): " << s << std::endl;
    // Expect the time portion "10:15:30" and the rest "1 UP 5" to appear.
    assert(s.find("10:15:30") != std::string::npos);
    assert(s.find("1 UP 5") != std::string::npos);
    
    // Test equality operator.
    ElevatorRequest req2(ts_str, 1, Direction::UP, 5);
    assert(req == req2);
    
    std::cout << "DTO tests passed." << std::endl;
    return 0;
}
