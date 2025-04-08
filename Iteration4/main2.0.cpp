#include <QApplication>
#include "ElevatorGUI.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ElevatorGUI gui;
    gui.show();
    return app.exec();
}
