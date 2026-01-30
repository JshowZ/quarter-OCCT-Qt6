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

class ProjectionWgt : public QMainWindow {
    Q_OBJECT
public:
    explicit ProjectionWgt(QWidget* parent = nullptr);
    ~ProjectionWgt();

private:
    QSplitter* m_splitter;
    SIM::Coin3D::Quarter::QuarterWidget* m_originalWidget;
    SIM::Coin3D::Quarter::QuarterWidget* m_projectionWidget;
    
    SoSeparator* m_originalRoot;
    SoSeparator* m_projectionRoot;
    
    SoPerspectiveCamera* m_originalCamera;
    SoOrthographicCamera* m_projectionCamera;
    
    SoSeparator* m_originalModelRoot;
    SoSeparator* m_projectionModelRoot;

    TopoDS_Shape m_originalShape;
    TopoDS_Shape m_projectionShape;

private:
    void setupUI();
    void createScene();
    SoSeparator* createOcctModel(TopoDS_Shape shape);
    void createOriginalShape();
    void createProjection();
    void setupCameras();
    void setupOriginalModel();
    void setupProjectionModel();
};
