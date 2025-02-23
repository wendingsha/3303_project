# Elevator Simulation

## Overview
This project simulates a elevator system where multiple elevators handle requests from different floors. The system assigns elevator requests efficiently, prioritizing idle elevators and selecting the closest available one when both are busy.

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

## How It Works
1. **Floor requests are generated** randomly.
2. **Scheduler assigns requests** to the nearest available elevator.
3. **Elevators process requests** by moving to pick up passengers and delivering them to their destination.
4. **The system runs for a fixed duration** before shutting down.

## Compilation & Execution
- Refer to the `execution.txt` file.


##Iteration 2 Updates
- This iteration implements the Scheduler and Elevator Subsystems, ensuring proper communication between the Floor Subsystem (which generates elevator requests) and the Elevator Subsystem (which processes requests).

###Features Implemented
- 'Floor Subsystem' : Reads input requests and sends them to the scheduler.
- 'Scheduler Subsystem' : Manages incoming requests and assigns them to the elevator.
- 'Elevator Subsystem' : Moves to requested floors, opens and closes doors, and updates system state.
- 'Synchronization' : Ensures only one request is processed at a time.
