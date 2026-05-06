#include <QApplication>
#include <QDebug>
#include <QByteArray>

#include "views/mainwindow.h"

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsEmpty("DISPLAY") && qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    }

    QApplication app(argc, argv);
    qInfo() << "Hello World from WireScope (app started)";
    qInfo() << "Qt platform:" << qgetenv("QT_QPA_PLATFORM");

    MainWindow window;
    window.show();

    return app.exec();
}
