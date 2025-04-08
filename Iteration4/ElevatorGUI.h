#ifndef ELEVATORGUI_H
#define ELEVATORGUI_H

#include <QMainWindow>
#include <QPushButton>
#include <QTextEdit>
#include <QThread>

class ElevatorGUI : public QMainWindow {
    Q_OBJECT

public:
    ElevatorGUI(QWidget *parent = nullptr);
    ~ElevatorGUI();

private slots:
    void handleStartSystem();
    void handleSendRequest();

private:
    QPushButton *startButton;
    QPushButton *requestButton;
    QTextEdit *logBox;

    void logMessage(const QString &msg);
};

#endif // ELEVATORGUI_H
