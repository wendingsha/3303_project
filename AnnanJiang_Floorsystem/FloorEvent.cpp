#include "FloorEvent.h"
#include <sstream>
#include <iostream>

// 默认构造函数（实现）
FloorEvent::FloorEvent() : timestamp(""), floorNumber(0), buttonType(""), carButton(0) {}

// 带参数的构造函数（实现）
FloorEvent::FloorEvent(const std::string& time, int floor, const std::string& button, int targetFloor)
    : timestamp(time), floorNumber(floor), buttonType(button), carButton(targetFloor) {}

// 将对象转换为字符串（实现）
std::string FloorEvent::serialize() const {
    std::ostringstream oss;
    oss << timestamp << "," << floorNumber << "," << buttonType << "," << carButton;
    return oss.str();
}

// 将字符串转换为对象（实现）
FloorEvent FloorEvent::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string timestamp, buttonType;
    int floorNumber = 0;
    int carButton = 0;
    char delimiter;

    // 解析时间戳
    if (!std::getline(iss, timestamp, ',')) {
        std::cerr << "Error: Failed to parse timestamp from '" << data << "'" << std::endl;
        return FloorEvent();
    }

    // 解析楼层号
    if (!(iss >> floorNumber)) {
        std::cerr << "Error: Failed to parse floorNumber from '" << data << "'" << std::endl;
        return FloorEvent();
    }

    // 跳过逗号
    if (!(iss >> delimiter) || delimiter != ',') {
        std::cerr << "Error: Missing or incorrect delimiter after floorNumber in '" << data << "'" << std::endl;
        return FloorEvent();
    }

    // 解析按钮类型
    if (!std::getline(iss, buttonType, ',')) {
        std::cerr << "Error: Failed to parse buttonType from '" << data << "'" << std::endl;
        return FloorEvent();
    }

    // 解析目标楼层号
    if (!(iss >> carButton)) {
        std::cerr << "Error: Failed to parse carButton from '" << data << "'" << std::endl;
        return FloorEvent();
    }

    // 返回解析后的 FloorEvent 对象
    return FloorEvent(timestamp, floorNumber, buttonType, carButton);
}
