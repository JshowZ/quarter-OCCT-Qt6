#pragma once

#include <QWidget>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <Quarter/QuarterWidget.h>
#include <Quarter/Quarter.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <TopoDS_Shape.hxx>
#include <AIS_Shape.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <Quantity_Color.hxx>
#include <QMouseEvent>

using namespace SIM::Coin3D::Quarter;

class QuarterViewer : public QWidget
{
    Q_OBJECT
public:
    explicit QuarterViewer(QWidget *parent = nullptr);
    ~QuarterViewer();
    
    // Initialize the view
    void initialize();
    
    // Add OCCT shape to the view
    void addShape(const TopoDS_Shape &shape, const Quantity_Color &color, int lineStyle, int lineWidth);
    
    // Clear all shapes
    void clearAllShapes();
    
    // Render the current scene
    void render();
    
    // Set current drawing mode (point, line, etc.)
    void setCurrentMode(int mode);
    
    // Get 3D point corresponding to current mouse position
    gp_Pnt getPointFromScreen(int x, int y);
    gp_Pnt getPointFromScreen(const QPoint &screenPos);

signals:
    // Mouse event signals
    void mousePressed(const gp_Pnt &point);
    void mouseMoved(const gp_Pnt &point);
    void mouseReleased(const gp_Pnt &point);
    
    // View update signal
    void viewUpdated();
    
protected:
    // Mouse event handling
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    
private:
    QuarterWidget *m_quarterWidget;
    SoSeparator *m_rootNode;
    SoSeparator *m_shapesRoot;
    V3d_Viewer *m_viewer3d;
    V3d_View *m_view3d;
    Handle(AIS_InteractiveContext) m_context;
    int m_currentMode;
    QPoint m_lastMousePos; // Save last mouse position
    
    // Convert OCCT shape to Coin3D node
    SoNode *shapeToCoinNode(const TopoDS_Shape &shape, const Quantity_Color &color, int lineStyle, int lineWidth);
};