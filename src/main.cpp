#include <QApplication>
#include <QSurfaceFormat>
#include <QDebug>
#include "mainwindow.h"
#include "AnotherMainWindow.h"
#include <Quarter/Quarter.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoCone.h>

int main(int argc, char *argv[])
{
    // Create Qt application instance
    QApplication a(argc, argv);
    
    // Load Chinese font support
    QFont font = a.font();
    font.setFamily("SimHei"); // Use SimHei font
    a.setFont(font);
    
    try {
        // Create and show main window
        AnotherMainWindow w;
        w.show();
        
        // Enter application event loop
        return a.exec();
    } catch (const std::exception &e) {
        qDebug() << "Exception caught:" << e.what();
        return 1;
    } catch (...) {
        qDebug() << "Unknown exception caught";
        return 1;
    }

    return 0;
}