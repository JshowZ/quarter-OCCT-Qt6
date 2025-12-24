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
private:
private:
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    SIM::Coin3D::Quarter::QuarterWidget* m_quarterWidget;
    SoSeparator* m_root;
    SoSeparator* m_modelRoot;

public:
    explicit CubeWindow(QWidget* parent = nullptr);
    ~CubeWindow();
    
private:
    void setupUI();
    void createCube();
};
