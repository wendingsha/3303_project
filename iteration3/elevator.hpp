#ifndef ELEVATOR_HPP
#define ELEVATOR_HPP

#include "message.hpp"

#include <vector>

extern std::vector<bool> elevatorBusy;

void elevatorFunction(int elevatorId);

#endif // ELEVATOR_HPP
