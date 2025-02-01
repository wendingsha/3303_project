#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

struct MyStruct
{
	int time;
	int floor;
	std::string button;
	int priority;

	bool operator<(const MyStruct& other) const {
		return priority < other.priority;
	}
};

std::priority_queue<MyStruct> floor_to_scheduler;
std::priority_queue<MyStruct> elevator_to_scheduler;
std::priority_queue<MyStruct> scheduler_to_floor;
std::priority_queue<MyStruct> scheduler_to_elevator;

std::mutex mtx;
std::condition_variable cv;

void floor_system() {
	MyStruct a = { 1, 3, "up", 2 };
	std::cout << "Floor subsystem sent message" << std::endl;

	{
		std::lock_guard<std::mutex> lock(mtx);
		floor_to_scheduler.push(a);
	}
	cv.notify_all();
	{
	std::unique_lock<std::mutex> lock(mtx);
	cv.wait(lock, [] {return !scheduler_to_floor.empty();});
	MyStruct response0 = scheduler_to_floor.top();
	scheduler_to_floor.pop();
	std::cout << "Floor received the response: Floor " << response0.floor << ", button " << response0.button << "processed." << std::endl;
	}
}

void scheduler_system() {
	std::unique_lock<std::mutex> lock(mtx);

	cv.wait(lock, [] {return !floor_to_scheduler.empty();});
	MyStruct response = floor_to_scheduler.top();
	floor_to_scheduler.pop();

	scheduler_to_elevator.push(response);
	cv.notify_all();

	cv.wait(lock, [] {return !elevator_to_scheduler.empty();});
	MyStruct response1 = elevator_to_scheduler.top();
	elevator_to_scheduler.pop();

	scheduler_to_floor.push(response1);
	cv.notify_all();
}

void elevactor_system() {
	std::unique_lock<std::mutex> lock(mtx);
	cv.wait(lock, [] {return !scheduler_to_elevator.empty();});
	MyStruct request = scheduler_to_elevator.top();
	scheduler_to_elevator.pop();

	std::cout << "Elevactor sent the request: Elevactor " << request.floor << ", button " << request.button << std::endl;
	elevator_to_scheduler.push(request);
	cv.notify_all();
}

int main() {
	std::thread floor_(floor_system);
	std::thread scheduler_(scheduler_system);
	std::thread elevactor_(elevactor_system);

	floor_.join();
	scheduler_.join();
	elevactor_.join();



	return 0;
}
