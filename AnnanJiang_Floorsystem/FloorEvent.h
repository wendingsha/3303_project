#ifndef FLOOREVENT_H
#define FLOOREVENT_H

#include <string>

class FloorEvent
{
public:
    std::string timestamp;  // 时间戳
    int floorNumber;        // 楼层号
    std::string buttonType; // 按钮类型
    int carButton;   

    FloorEvent();

    // construct
    FloorEvent(const std::string& time, int floor, const std::string& button, int targetFloor);

    // turn object to string
    std::string serialize() const;

    // turn string to object
    static FloorEvent deserialize(const std::string& data);
};

#endif