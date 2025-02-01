#include "InputParser.h"
#include "FloorEvent.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <queue>

std::queue<FloorEvent> InputParser::parseFile(const std::string& fileName) {
    std::queue<FloorEvent> eventQueue; // Queue to store parsed FloorEvent objects
    std::ifstream file(fileName);     // Open the input file

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << fileName << std::endl;
        return eventQueue; // Return an empty queue if the file cannot be opened
    }

    std::string line;
    while (std::getline(file, line)) { // Read each line from the file
        if (line.empty()) { // Skip empty lines
            continue;
        }

        std::istringstream iss(line); // Create a string stream for parsing the line
        std::string timestamp, buttonType;
        int floorNumber = 0, carButton = 0;
        char delimiter;

        // Parse the line using ',' as a delimiter
        if (std::getline(iss, timestamp, ',') &&  // Parse timestamp
            (iss >> floorNumber) &&               // Parse floor number
            (iss >> delimiter) && delimiter == ',' &&  // Validate and skip delimiter
            std::getline(iss, buttonType, ',') && // Parse button type
            (iss >> carButton)) {                 // Parse car button
            // Successfully parsed all fields, add the event to the queue
            eventQueue.emplace(timestamp, floorNumber, buttonType, carButton);
        } else {
            // Handle invalid line format
            std::cerr << "Warning: Invalid line format: " << line << std::endl;
        }
    }

    file.close(); // Close the file
    return eventQueue; // Return the queue with parsed events
}
