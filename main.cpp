#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Live Captions Controller");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("YourCompany");
    
    // Set fusion style for better look
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Optional: Set dark theme
    QFile styleFile(":/styles/dark.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        app.setStyleSheet(styleSheet);
    }
    
    MainWindow window;
    window.show();
    
    return app.exec();
}