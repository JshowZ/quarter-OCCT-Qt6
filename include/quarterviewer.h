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
    
    // 初始化视图
    void initialize();
    
    // 添加OCCT形状到视图
    void addShape(const TopoDS_Shape &shape, const Quantity_Color &color, int lineStyle, int lineWidth);
    
    // 清除所有形状
    void clearAllShapes();
    
    // 渲染当前场景
    void render();
    
    // 设置当前绘制模式（点、线等）
    void setCurrentMode(int mode);
    
    // 获取当前鼠标位置对应的3D点
    gp_Pnt getPointFromScreen(int x, int y);
    gp_Pnt getPointFromScreen(const QPoint &screenPos);

signals:
    // 鼠标事件信号
    void mousePressed(const gp_Pnt &point);
    void mouseMoved(const gp_Pnt &point);
    void mouseReleased(const gp_Pnt &point);
    
    // 视图更新信号
    void viewUpdated();
    
protected:
    // 鼠标事件处理
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
    QPoint m_lastMousePos; // 保存上次鼠标位置
    
    // 将OCCT形状转换为Coin3D节点
    SoNode *shapeToCoinNode(const TopoDS_Shape &shape, const Quantity_Color &color, int lineStyle, int lineWidth);
};