#include <QApplication>
#include "TestWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Load Chinese font support
    QFont font = a.font();
    font.setFamily("SimHei"); // Use SimHei font for Chinese characters
    a.setFont(font);
    
    // Create TestWindow
    TestWindow w;
    
    // Initialize the window
    w.initialize();
    
    // Draw 3D text at different positions
    w.draw3DText("Hello 3D Text", 0, 0, 0, 0.5f);
    w.draw3DText("X Axis", 2, 0, 0, 0.3f);
    w.draw3DText("Y Axis", 0, 2, 0, 0.3f);
    w.draw3DText("Z Axis", 0, 0, 2, 0.3f);
    w.draw3DText("三维文本测试", -2, 0, 0, 0.4f);
    
    // Show the window
    w.show();
    
    return a.exec();
}
