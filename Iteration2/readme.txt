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
