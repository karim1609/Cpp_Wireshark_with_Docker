#include <QApplication>
#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QStringList>

#include "views/mainwindow.h"

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsEmpty("DISPLAY") && qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    }

    QApplication app(argc, argv);
    qInfo() << "Hello World from WireScope (app started)";
    qInfo() << "Qt platform:" << qgetenv("QT_QPA_PLATFORM");

    const QStringList logoCandidates = {
        // Path of the logo you shared in this session.
        "C:/Users/Karim erradi/.cursor/projects/c-Users-Karim-erradi-Downloads-wirshark-with-docker/assets/c__Users_Karim_erradi_AppData_Roaming_Cursor_User_workspaceStorage_ad558b227f374614ca81ca4fd884b559_images_image-ad7d06b1-06c3-4d64-afce-7b197a9facde.png",
        QDir::currentPath() + "/assets/netscope_logo.png",
        QDir::currentPath() + "/assets/logo.png",
        "/app/assets/netscope_logo.png",
        "/app/assets/logo.png"
    };

    QIcon appLogo;
    for (const QString &candidate : logoCandidates) {
        if (QFileInfo::exists(candidate)) {
            appLogo = QIcon(candidate);
            qInfo() << "Using app logo:" << candidate;
            break;
        }
    }
    if (!appLogo.isNull()) {
        app.setWindowIcon(appLogo);
    } else {
        qWarning() << "No app logo file found. Using default icon.";
    }

    MainWindow window;
    if (!appLogo.isNull()) {
        window.setWindowIcon(appLogo);
    }
    window.show();

    return app.exec();
}
