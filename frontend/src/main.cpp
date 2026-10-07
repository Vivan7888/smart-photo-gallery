#include "MainWindow.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Smart Photo Gallery Team");
    QCoreApplication::setApplicationName("Smart Photo Gallery");

    MainWindow window;
    window.show();
    return app.exec();
}
