#ifndef INPUTPARSER_H
#define INPUTPARSER_H

#include <string>
#include <queue>
#include "FloorEvent.h" 

class InputParser {
public:
    // 解析输入文件并返回事件队列
    static std::queue<FloorEvent> parseFile(const std::string& fileName);
};

#endif // INPUTPARSER_H
