#include <QApplication>
#include <QSurfaceFormat>
#include <QDebug>
#include "mainwindow.h"
#include <Quarter/Quarter.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoCone.h>

int main(int argc, char *argv[])
{
    // 设置OpenGL格式
    QSurfaceFormat format;
    format.setVersion(4, 1); // 设置OpenGL版本
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples(4);    // 开启抗锯齿
    QSurfaceFormat::setDefaultFormat(format);
    
    // 创建Qt应用程序实例
    QApplication a(argc, argv);
    
    // 加载中文字体支持
    QFont font = a.font();
    font.setFamily("SimHei"); // 使用黑体字体
    a.setFont(font);
    
    try {
        // 创建并显示主窗口
        MainWindow w;
        w.show();
        
        // 进入应用程序的事件循环
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