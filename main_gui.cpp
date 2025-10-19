#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application information
    QApplication::setApplicationName("GWA");
    QApplication::setApplicationVersion("1.0");
    QApplication::setOrganizationName("EnviroInformatics LLC");
    QApplication::setApplicationDisplayName("Groundwater Age Modeling");

    // Create and show main window
    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
