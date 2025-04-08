#ifndef ELEVATORGUI_H
#define ELEVATORGUI_H

#include <QMainWindow>
#include <QPushButton>
#include <QTextEdit>
#include <QThread>

/**
 * ElevatorGUI is the main window class for the elevator system GUI.
 * It provides buttons to start the elevator system and simulate requests.
 * It also includes a text area to display system logs.
 */
class ElevatorGUI : public QMainWindow {
    Q_OBJECT

public:
    ElevatorGUI(QWidget *parent = nullptr);
    ~ElevatorGUI();

private slots:
    // Called when the user clicks the "Start System" button
    void handleStartSystem();

    // Called when the user clicks the "Send Request" button (future extension)
    void handleSendRequest();

private:
    QPushButton *startButton;    // Button to start the system
    QPushButton *requestButton;  // Button to simulate request (optional)
    QTextEdit *logBox;           // Text area to show system logs

    // Appends a message to the logBox with a timestamp
    void logMessage(const QString &msg);
};

#endif // ELEVATORGUI_H
