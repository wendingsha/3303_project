#include "ElevatorGUI.h"
#include <QVBoxLayout>
#include <QDateTime>
#include <thread>
#include "floor.cpp"
#include "scheduler.cpp"
#include "elevator.cpp"

// 全局变量定义
std::atomic<int> currentTime(0);
std::mutex printMutex;
bool systemActive = true;

ElevatorGUI::ElevatorGUI(QWidget *parent)
    : QMainWindow(parent) {

    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);

    startButton = new QPushButton("启动系统");
    requestButton = new QPushButton("请求电梯");
    logBox = new QTextEdit();
    logBox->setReadOnly(true);

    layout->addWidget(startButton);
    layout->addWidget(requestButton);
    layout->addWidget(logBox);
    setCentralWidget(central);

    connect(startButton, &QPushButton::clicked, this, &ElevatorGUI::handleStartSystem);
    connect(requestButton, &QPushButton::clicked, this, &ElevatorGUI::handleSendRequest);

    setWindowTitle("电梯系统 GUI 控制器");
    resize(400, 300);
}

ElevatorGUI::~ElevatorGUI() {}

void ElevatorGUI::logMessage(const QString &msg) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    logBox->append("[" + timestamp + "] " + msg);
}

void ElevatorGUI::handleStartSystem() {
    logMessage("系统启动中...");

    std::thread schedulerThread([]() {
        schedulerFunction();
    });

    std::thread elevator0([]() {
        elevatorFunction(0);
    });

    std::thread elevator1([]() {
        elevatorFunction(1);
    });

    std::thread floorThread([]() {
        floorFunction();
    });

    schedulerThread.detach();
    elevator0.detach();
    elevator1.detach();
    floorThread.detach();

    logMessage("所有模块已启动！");
}

void ElevatorGUI::handleSendRequest() {
    logMessage("模拟发送请求：input.txt 会被 floorFunction 读取并触发调度。");
}
