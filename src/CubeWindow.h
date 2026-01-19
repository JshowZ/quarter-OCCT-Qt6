#pragma once

#include <QMainWindow>

// Forward declarations
class QWidget;
class QVBoxLayout;
class QMouseEvent;
class SoSeparator;
class SoPerspectiveCamera;
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
    
    /**
     * @brief Handle mouse press events for picking
     * @param event Mouse press event
     */
    void mousePressEvent(QMouseEvent* event) override;

private:
    SIM::Coin3D::Quarter::QuarterWidget* m_quarterWidget;
    SoSeparator* m_root;
    SoSeparator* m_modelRoot;
    SoPerspectiveCamera* m_camera;
    SoSeparator* m_pickRoot;
    bool m_pickEnabled;

    
private:
    void setupUI();
    void createCube();
    /**
     * @brief Perform picking operation at given mouse position
     * @param x Mouse X position
     * @param y Mouse Y position
     */
    void performPick(int x, int y);
};
