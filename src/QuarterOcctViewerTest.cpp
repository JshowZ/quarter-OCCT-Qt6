#include <QApplication>
#include <QSurfaceFormat>
#include <Quarter/Quarter.h>
#include "QuarterOcctViewer.h"

int main(int argc, char *argv[])
{
    // Set OpenGL format
    QSurfaceFormat format;
    format.setVersion(4, 1); // Set OpenGL version
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples(4);    // Enable anti-aliasing
    QSurfaceFormat::setDefaultFormat(format);
    
    // Create Qt application instance
    QApplication a(argc, argv);
    
    // Initialize Quarter
    SIM::Coin3D::Quarter::Quarter::init();
    
    // Create and show the QuarterOcctViewer window
    QuarterOcctViewer viewer;
    viewer.show();
    
    // Enter application event loop
    return a.exec();
}