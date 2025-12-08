//#include <QApplication>
//#include <QSurfaceFormat>
//#include <QDebug>
//#include "mainwindow.h"
//#include "AnotherMainWindow.h"
//#include <Quarter/Quarter.h>
//#include <Inventor/nodes/SoBaseColor.h>
//#include <Inventor/nodes/SoCone.h>
//
//int main(int argc, char *argv[])
//{
//    // Set OpenGL format
//    QSurfaceFormat format;
//    format.setVersion(4, 1); // Set OpenGL version
//    format.setProfile(QSurfaceFormat::CoreProfile);
//    format.setSamples(4);    // Enable anti-aliasing
//    QSurfaceFormat::setDefaultFormat(format);
//    
//    // Create Qt application instance
//    QApplication a(argc, argv);
//    
//    // Load Chinese font support
//    QFont font = a.font();
//    font.setFamily("SimHei"); // Use SimHei font
//    a.setFont(font);
//    
//    try {
//        // Create and show main window
//        AnotherMainWindow w;
//        w.show();
//        
//        // Enter application event loop
//        return a.exec();
//    } catch (const std::exception &e) {
//        qDebug() << "Exception caught:" << e.what();
//        return 1;
//    } catch (...) {
//        qDebug() << "Unknown exception caught";
//        return 1;
//    }
//
//    return 0;
//}

#include <Quarter/QuarterWidget.h>
#include <Quarter/Quarter.h>
#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <AIS_Shape.hxx>
#include <BRepTools.hxx>
#include <BRepBuilderAPI.hxx>
#include <TopoDS_Shape.hxx>

#include <QApplication>

using namespace SIM::Coin3D::Quarter;

int main(int argc,char *argv[])
{
	Quarter::init();

	QApplication app(argc, argv);

	QuarterWidget quarterWidget;
	quarterWidget.setWindowTitle("QuarterWidget Example");
	quarterWidget.show();

	Handle(V3d_View) view = quarterWidget.getView();
}
