#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "message.hpp"
#include <vector>
#include <queue> // Add this include for std::queue



extern std::vector<bool> elevatorBusy; // Declare as extern

void schedulerFunction();
void displayDashboard();

#endif // SCHEDULER_HPP