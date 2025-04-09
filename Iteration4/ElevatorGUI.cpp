#include "ElevatorGUI.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QDateTime>
#include <thread>
#include "floor.cpp"
#include "scheduler.cpp"
#include "elevator.cpp"
#include "shared.hpp"

std::atomic<int> currentTime(0);
std::mutex printMutex;
bool systemActive = true;

ElevatorGUI::ElevatorGUI(QWidget *parent)
    : QMainWindow(parent) {

    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);

    startButton = new QPushButton("Start System");
    statusTable = new QTableWidget(2, 5);
    statusTable->setHorizontalHeaderLabels({"Elevator", "Current Floor", "Status", "Destination", "Fault"});
    logBox = new QTextEdit();
    logBox->setReadOnly(true);

    layout->addWidget(startButton);
    layout->addWidget(statusTable);
    layout->addWidget(logBox);
    setCentralWidget(central);

    connect(startButton, &QPushButton::clicked, this, &ElevatorGUI::handleStartSystem);

    // Auto refresh every second
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &ElevatorGUI::refreshStatusTable);
    updateTimer->start(1000);

    setWindowTitle("Elevator System Monitor");
    resize(600, 400);
}

ElevatorGUI::~ElevatorGUI() {}

void ElevatorGUI::logMessage(const QString &msg) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    logBox->append("[" + timestamp + "] " + msg);
}

void ElevatorGUI::handleStartSystem() {
    logMessage("System starting...");

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

    logMessage("All components started.");
}

void ElevatorGUI::refreshStatusTable() {
    std::lock_guard<std::mutex> lock(statusMutex);
    for (int i = 0; i < elevatorStatuses.size(); ++i) {
        const ElevatorStatus &e = elevatorStatuses[i];
        statusTable->setItem(i, 0, new QTableWidgetItem(QString::number(e.id)));
        statusTable->setItem(i, 1, new QTableWidgetItem(QString::number(e.currentFloor)));
        statusTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(e.status)));
        statusTable->setItem(i, 3, new QTableWidgetItem(e.destination == -1 ? "-" : QString::number(e.destination)));
        statusTable->setItem(i, 4, new QTableWidgetItem(e.fault ? "Yes" : "No"));
    }
}
