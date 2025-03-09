# Elevator Simulation

## Overview
This project simulates an elevator system where multiple elevators handle requests from different floors. The system assigns elevator requests efficiently, prioritizing idle elevators and selecting the closest available one when both are busy.

## Features
- **Multiple Elevators:** Two elevators operate concurrently to handle floor requests.
- **Smart Scheduling:** Requests are assigned to idle elevators first, and if all are busy, the closest one is selected.
- **Threaded Simulation:** Uses multithreading with `std::thread`, `std::mutex`, and `std::condition_variable` for synchronization.
- **Real-Time Request Handling:** New requests are generated dynamically and distributed efficiently.

## Components
- `main.cpp` - Initializes and runs the simulation.
- `floor.cpp` - Generates random floor requests.
- `scheduler.cpp` - Assigns requests to the appropriate elevator.
- `elevator.cpp` - Handles elevator movement and request processing.
- `message.hpp` - Defines the message structure used for communication.
- `time_manager.cpp` - Manages the simulation time.

## Testing
The system includes unit tests for each major component:

- `elevator_simple_test.cpp` - Tests the elevator component functionality.
- `scheduler_simple_test.cpp` - Tests the scheduler component functionality.
- `floor_simple_test.cpp` - Tests the floor component functionality.

### Test Features
- Independent verification of each system component
- Detailed output showing each test case result
- Validation of core algorithmic logic
- No external dependencies (no Google Test required)

## How It Works
1. **Floor requests are generated** randomly.
2. **Scheduler assigns requests** to the nearest available elevator.
3. **Elevators process requests** by moving to pick up passengers and delivering them to their destination.
4. **The system runs for a fixed duration** before shutting down.

## Compilation & Execution

### Main Program
- Refer to the `execution.txt` file for main program compilation and execution.

### Unit Tests
To compile and run the unit tests:

# Compile the elevator test
g++ -std=c++17 elevator_simple_test.cpp -o elevator_test

# Compile the scheduler test
g++ -std=c++17 scheduler_simple_test.cpp -o scheduler_test

# Compile the floor test
g++ -std=c++17 floor_simple_test.cpp -o floor_test

# Run the tests
./elevator_test
./scheduler_test
./floor_test
