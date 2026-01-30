#pragma once

#include <QMainWindow>
#include <TopoDS_Shape.hxx>

class QWidget;
class QHBoxLayout;
class QVBoxLayout;
class QSplitter;
class SoSeparator;
class SoPerspectiveCamera;
class SoOrthographicCamera;
namespace SIM {
namespace Coin3D {
namespace Quarter {
    class QuarterWidget;
}
}
}

class TwoProjectionWidget : public QMainWindow {
    Q_OBJECT
public:
    explicit TwoProjectionWidget(QWidget* parent = nullptr);
    ~TwoProjectionWidget();

private:
    QSplitter* m_splitter;
    SIM::Coin3D::Quarter::QuarterWidget* m_leftWidget;
    SIM::Coin3D::Quarter::QuarterWidget* m_rightWidget;
    
    SoSeparator* m_leftRoot;
    SoSeparator* m_rightRoot;
    
    SoPerspectiveCamera* m_leftCamera;
    SoOrthographicCamera* m_rightCamera;
    
    SoSeparator* m_leftModelRoot;
    SoSeparator* m_rightModelRoot;

    TopoDS_Shape m_leftShape;
    TopoDS_Shape m_rightShape;

private:
    void setupUI();
    void createScene();
    SoSeparator* createOcctModel(TopoDS_Shape shape);
    void createOcctShapes();
    void setupCameras();
    void setupLeftModel();
    void setupRightModel();
};
