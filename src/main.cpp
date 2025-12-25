#include <iostream>
#include <QApplication>
#include <QSurfaceFormat>
#include <QFont>
#include <QDebug>

#include <Inventor/nodes/SoDepthBuffer.h>
#include <Inventor/SoDB.h>
#include <Inventor/SoInteraction.h>

#include <Quarter/Quarter.h>

#include "AnotherMainWindow.h"
#include "QuarterOcctViewer.h"
using namespace std;

int main(int argc, char** argv)
{
    // Set OpenGL format
     //QSurfaceFormat format;
     //format.setVersion(4, 1); // Set OpenGL version
     //format.setProfile(QSurfaceFormat::CoreProfile);
     //format.setSamples(4);    // Enable anti-aliasing
     //QSurfaceFormat::setDefaultFormat(format);
    
    // Create Qt application instance
     QApplication a(argc, argv);
    
    // Load Chinese font support
     QFont font = a.font();
     font.setFamily("SimHei"); // Use SimHei font
     a.setFont(font);
    
     try {
         // Create and show main window
         SoDB::init();
         SIM::Coin3D::Quarter::Quarter::init();
         //SoInteraction::init();
         //SoFCDB::init();
         QuarterOcctViewer w;
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
}