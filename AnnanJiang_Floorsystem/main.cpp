#include <iostream>
#include "InputParser.h"
#include "FloorEvent.h"
#include <queue>

int main() {
    // 测试输入文件
    std::string fileName = "input.txt";

    // 解析文件
    std::queue<FloorEvent> events = InputParser::parseFile(fileName);

    // 打印解析的事件
    while (!events.empty()) {
        FloorEvent event = events.front();
        events.pop();
        std::cout << "Event: " << event.timestamp << ", Floor: " << event.floorNumber
                  << ", Button: " << event.buttonType <<", Carbutton: "<<event.carButton<< std::endl;
    }

    return 0;
}
