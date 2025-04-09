#ifndef ELEVATORGUI_H
#define ELEVATORGUI_H

#include <QMainWindow>
#include <QPushButton>
#include <QTextEdit>
#include <QTableWidget>
#include <QTimer>

class ElevatorGUI : public QMainWindow {
    Q_OBJECT

public:
    ElevatorGUI(QWidget *parent = nullptr);
    ~ElevatorGUI();

private slots:
    void handleStartSystem();
    void refreshStatusTable();

private:
    QPushButton *startButton;
    QTableWidget *statusTable;
    QTextEdit *logBox;
    QTimer *updateTimer;

    void logMessage(const QString &msg);
};

#endif // ELEVATORGUI_H
