#pragma once

#include <QWidget>
#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <TopoDS_Shape.hxx>
#include <WNT_Window.hxx>

class OccWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OccWidget(QWidget* parent = nullptr);
    ~OccWidget();

    void displayShape(const TopoDS_Shape& shape);
    void fitAll();
    void eraseAll();

protected:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void initOCC();
    void updateView();

    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View) m_view;
    Handle(AIS_InteractiveContext) m_context;
    Handle(WNT_Window) m_wntWindow;

    bool m_isInitialized;
    QPoint m_lastMousePos;
    bool m_isRotating;
    bool m_isPanning;
};

