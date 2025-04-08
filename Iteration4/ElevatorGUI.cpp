#include "ElevatorGUI.h"
#include <QVBoxLayout>
#include <QDateTime>
#include <thread>
#include "floor.cpp"
#include "scheduler.cpp"
#include "elevator.cpp"

// Global variables shared across modules
std::atomic<int> currentTime(0);
std::mutex printMutex;
bool systemActive = true;

/**
 * Constructor for ElevatorGUI
 * Sets up the UI layout and connects button signals to their slots
 */
ElevatorGUI::ElevatorGUI(QWidget *parent)
    : QMainWindow(parent) {

    // Set up central widget and layout
    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);

    // Create buttons and log display
    startButton = new QPushButton("Start System");
    requestButton = new QPushButton("Send Request");
    logBox = new QTextEdit();
    logBox->setReadOnly(true);

    // Add widgets to layout
    layout->addWidget(startButton);
    layout->addWidget(requestButton);
    layout->addWidget(logBox);
    setCentralWidget(central);

    // Connect button clicks to their respective slot functions
    connect(startButton, &QPushButton::clicked, this, &ElevatorGUI::handleStartSystem);
    connect(requestButton, &QPushButton::clicked, this, &ElevatorGUI::handleSendRequest);

    setWindowTitle("Elevator System GUI Controller");
    resize(400, 300);
}

/**
 * Destructor for ElevatorGUI
 */
ElevatorGUI::~ElevatorGUI() {}

/**
 * Appends a message to the log window with timestamp
 */
void ElevatorGUI::logMessage(const QString &msg) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    logBox->append("[" + timestamp + "] " + msg);
}

/**
 * Starts the elevator system by launching each component in a separate thread
 */
void ElevatorGUI::handleStartSystem() {
    logMessage("Starting elevator system...");

    // Start Scheduler
    std::thread schedulerThread([]() {
        schedulerFunction();
    });

    // Start two elevators
    std::thread elevator0([]() {
        elevatorFunction(0);
    });

    std::thread elevator1([]() {
        elevatorFunction(1);
    });

    // Start floor simulation (reads from input.txt)
    std::thread floorThread([]() {
        floorFunction();
    });

    // Detach all threads so they run independently
    schedulerThread.detach();
    elevator0.detach();
    elevator1.detach();
    floorThread.detach();

    logMessage("All components started.");
}

/**
 * Simulates sending a request (currently just logs a message)
 * You can extend this to manually trigger requests
 */
void ElevatorGUI::handleSendRequest() {
    logMessage("Simulated request (input.txt will be used by floorFunction).");
}
