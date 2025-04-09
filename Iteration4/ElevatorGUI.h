#ifndef ELEVATORGUI_H
#define ELEVATORGUI_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTimer>

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
