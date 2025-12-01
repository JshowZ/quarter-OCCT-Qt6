#pragma once

#include <QOpenGLWidget>
#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <TopoDS_Shape.hxx>

class OccWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit OccWidget(QWidget* parent = nullptr);
    ~OccWidget();

    void displayShape(const TopoDS_Shape& shape);
    void fitAll();
    void eraseAll();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void initOCC();

    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View) m_view;
    Handle(AIS_InteractiveContext) m_context;

    QPoint m_lastMousePos;
    bool m_isRotating;
    bool m_isPanning;
};

