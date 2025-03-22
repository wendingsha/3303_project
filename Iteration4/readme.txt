# Elevator Simulation (Iteration 4)

## Overview
This project simulates a multi-elevator system handling floor requests in real time. It features dynamic scheduling, continuous position updates, and fault detection/correction to mimic realistic elevator behavior.

## Features
- **Multiple Elevators:** Two elevators operate concurrently.
- **Smart Scheduling:** Requests are assigned to idle or the nearest available elevator.
- **Fault Injection & Detection:** Faults (door or stuck faults) can be injected via the input file; the scheduler detects timeouts and reassigns requests from faulted elevators.
- **Real-Time Updates:** Elevators send intermediate floor-by-floor updates to the scheduler.
- **Multithreaded Simulation:** Utilizes `std::thread`, `std::mutex`, and condition synchronization for real-time operation.

## Components
- **main.cpp:** Initializes and runs the simulation.
- **floor.cpp:** Reads floor requests (with optional fault codes) from an input file and generates requests.
- **scheduler.cpp:** Assigns requests based on real-time elevator status, monitors for faults, and reassigns requests as needed.
- **elevator.cpp:** Processes requests by moving floor-by-floor, sends intermediate updates, and simulates faults.
- **message.hpp:** Defines the message structure for communication among components.

## How It Works
1. **Request Generation:** Floor requests (with optional fault codes) are read from an input file.
2. **Assignment:** The scheduler assigns requests to the nearest available, non-faulted elevator.
3. **Processing:** Elevators move floor-by-floor, sending updates and simulating faults when indicated.
4. **Fault Handling:** The scheduler monitors for timeouts, marks unresponsive elevators as faulted, and reassigns affected requests.

## Compilation & Execution
Compile with:
```bash
g++ -std=c++11 -pthread main.cpp elevator.cpp floor.cpp scheduler.cpp time_manager.cpp -o elevator_sim
Run the Simulation:
./elevator_sim
