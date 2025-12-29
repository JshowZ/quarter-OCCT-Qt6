#pragma once

#include <QMainWindow>

// Forward declarations
class QWidget;
class QVBoxLayout;
class SoSeparator;
namespace SIM {
namespace Coin3D {
namespace Quarter {
    class QuarterWidget;
}
}
}

class CubeWindow : public QMainWindow {
public:
    explicit CubeWindow(QWidget* parent = nullptr);
    ~CubeWindow();

private:
    SIM::Coin3D::Quarter::QuarterWidget* m_quarterWidget;
    SoSeparator* m_root;
    SoSeparator* m_modelRoot;


    
private:
    void setupUI();
    void createCube();
};
