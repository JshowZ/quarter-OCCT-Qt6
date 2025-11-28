#include "quarterviewer.h"
#include <QVBoxLayout>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/manips/SoTrackballManip.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoCone.h>
#include <QMessageBox>

// OCCT相关头文件
#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <WNT_Window.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <StdPrs_Point.hxx>
#include <StdPrs_WFShape.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_LineAspect.hxx>
#include <BRep_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS.hxx>
#include <gp_Pnt.hxx>

QuarterViewer::QuarterViewer(QWidget *parent) : QWidget(parent)
{
    // 设置窗口属性，确保OpenGL正确渲染
    this->setAttribute(Qt::WA_PaintOnScreen);
    this->setAttribute(Qt::WA_OpaquePaintEvent);
    this->setAttribute(Qt::WA_NoSystemBackground);
    this->setFocusPolicy(Qt::StrongFocus);
    
    // 创建布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // 初始化Quarter
    Quarter::init();
    
    // 创建QuarterWidget，设置关键属性
    m_quarterWidget = new QuarterWidget(this);
    m_quarterWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_quarterWidget->setFocusPolicy(Qt::StrongFocus);
    layout->addWidget(m_quarterWidget);
    
    qDebug() << "QuarterViewer: Widget created with OpenGL attributes";
    qDebug() << "QuarterViewer: Quarter initialized";
    qDebug() << "QuarterViewer: QuarterWidget created";
    
    // 初始化其他成员
    m_rootNode = new SoSeparator();
    m_rootNode->ref();
    
    m_shapesRoot = new SoSeparator();
    m_shapesRoot->ref();

    SoBaseColor* col = new SoBaseColor;
    col->rgb = SbColor(1, 1, 0);
    m_rootNode->addChild(col);

    m_rootNode->addChild(new SoCone);
    //m_rootNode->addChild(m_shapesRoot);
    //
    //// 创建相机
    //SoPerspectiveCamera *camera = new SoPerspectiveCamera();
    //camera->position.setValue(0, 0, 200); // 增加相机距离，扩大视野
    //camera->orientation.setValue(0, 0, 0, 1);
    //camera->heightAngle = M_PI / 3; // 增大视角，看到更多内容
    //camera->nearDistance = 1;
    //camera->farDistance = 1000;
    //m_rootNode->addChild(camera);
    //
    //// 添加灯光
    //SoDirectionalLight *light = new SoDirectionalLight();
    //light->direction.setValue(-1, -1, -1);
    //light->intensity.setValue(0.8);
    //m_rootNode->addChild(light);
    //
    //// 添加交互控件，允许用户旋转、缩放和平移视图
    //SoTrackballManip *manipulator = new SoTrackballManip();
    //m_rootNode->addChild(manipulator);
    //
    //qDebug() << "QuarterViewer: Camera position:" << camera->position.getValue()[0] << "," << camera->position.getValue()[1] << "," << camera->position.getValue()[2];
    //
    //// 设置场景图
    //m_quarterWidget->setSceneGraph(m_rootNode);
    //
    //m_currentMode = 0;
}

QuarterViewer::~QuarterViewer()
{
    m_shapesRoot->unref();
    m_rootNode->unref();
    delete m_quarterWidget;
    //Quarter::exit();
}

void QuarterViewer::initialize()
{
    //// 设置初始视图
    //m_quarterWidget->viewAll();
    //
    //// 确保QuarterWidget正确显示
    //this->setAttribute(Qt::WA_OpaquePaintEvent);
    //this->setAttribute(Qt::WA_PaintOnScreen);
    //
    //// 简化背景设置，先移除可能导致问题的代码
    //
    //// 添加明显的测试直线，增大线条宽度和颜色对比度
    //qDebug() << "QuarterViewer: Initializing with test geometry";
    //
    //// 添加测试直线，验证绘制功能
    //SoSeparator *testSeparator = new SoSeparator();
    //
    //// 设置材料 - 使用更鲜艳的颜色
    //SoMaterial *testMaterial = new SoMaterial();
    //testMaterial->diffuseColor.setValue(1.0, 0.0, 0.0); // 鲜艳的红色
    //testMaterial->emissiveColor.setValue(0.5, 0.0, 0.0); // 增加自发光，提高可见性
    //testSeparator->addChild(testMaterial);
    //
    //// 设置绘制样式 - 增大线条宽度
    //SoDrawStyle *testDrawStyle = new SoDrawStyle();
    //testDrawStyle->lineWidth.setValue(5.0); // 更粗的线条
    //testSeparator->addChild(testDrawStyle);
    //
    //// 创建多条直线，形成一个十字，增加可见性
    //SoCoordinate3 *testCoords = new SoCoordinate3();
    //// 水平线
    //testCoords->point.set1Value(0, -100, 0, 0); // 左端点
    //testCoords->point.set1Value(1, 100, 0, 0);  // 右端点
    //// 垂直线
    //testCoords->point.set1Value(2, 0, -100, 0); // 下端点
    //testCoords->point.set1Value(3, 0, 100, 0);  // 上端点
    //testSeparator->addChild(testCoords);
    //
    //// 创建线条集
    //SoLineSet *testLineSet = new SoLineSet();
    //// 指定每条线的顶点数
    //int numVertices[] = {2, 2}; // 两条线，每条2个顶点
    //testLineSet->numVertices.setValues(0, 2, numVertices);
    //testSeparator->addChild(testLineSet);
    //
    //// 确保测试图形位于场景的中心和可见区域
    //SoTransform *transform = new SoTransform();
    //transform->translation.setValue(0, 0, 0);
    //transform->scaleFactor.setValue(1, 1, 1);
    //testSeparator->insertChild(transform, 0);
    //
    //// 清除之前可能存在的形状，确保只显示测试图形
    //m_shapesRoot->removeAllChildren();
    //
    //// 添加到场景
    //m_shapesRoot->addChild(testSeparator);
    //
    //// 添加额外的调试信息
    //qDebug() << "QuarterViewer: Test cross added to scene with lines: horizontal from (-100,0,0) to (100,0,0) and vertical from (0,-100,0) to (0,100,0)";
    //qDebug() << "QuarterViewer: Camera position:" << m_rootNode->getChild(3)->getTypeId().getName(); // 检查相机节点
    //
    //// 强制更新视图
    //m_quarterWidget->update();
    //this->update();
    qDebug() << "QuarterViewer: View updated after adding test geometry";
}

void QuarterViewer::addShape(const TopoDS_Shape &shape, const Quantity_Color &color, int lineStyle, int lineWidth)
{
    try {
        // 检查形状是否有效
        if (shape.IsNull()) {
            qDebug() << "QuarterViewer: Attempting to add null shape, skipping";
            return;
        }
        
        // 将OCCT形状转换为Coin3D节点
        SoNode *node = shapeToCoinNode(shape, color, lineStyle, lineWidth);
        
        // 如果转换成功，添加到根节点
        if (node) {
            m_shapesRoot->addChild(node);
            qDebug() << "QuarterViewer: Shape added to scene graph";
            emit viewUpdated();
        } else {
            qDebug() << "QuarterViewer: Failed to convert shape to Coin3D node";
        }
    } catch (const Standard_Failure &e) {
        qDebug() << "QuarterViewer: OCCT exception when adding shape:" << e.GetMessageString();
    } catch (...) {
        qDebug() << "QuarterViewer: Unknown exception when adding shape";
    }
}

void QuarterViewer::clearAllShapes()
{
    if (m_shapesRoot) {
        m_shapesRoot->removeAllChildren();
        qDebug() << "QuarterViewer: All shapes cleared from scene graph";
    }
    emit viewUpdated();
}

void QuarterViewer::render()
{
    // QuarterWidget没有updateGL方法，使用update()替代
    m_quarterWidget->update();
}

void QuarterViewer::setCurrentMode(int mode)
{
    m_currentMode = mode;
}

gp_Pnt QuarterViewer::getPointFromScreen(const QPoint &screenPos)
{
    // 计算屏幕坐标对应的3D点
    // 这里使用简化的实现，实际项目中可能需要更复杂的射线投射
    
    // 获取视口大小
    QSize viewportSize = m_quarterWidget->size();
    
    // 归一化坐标到[-1, 1]范围
    Standard_Real normalizedX = (2.0 * screenPos.x() / viewportSize.width()) - 1.0;
    Standard_Real normalizedY = 1.0 - (2.0 * screenPos.y() / viewportSize.height());
    
    // 假设一个简单的正交投影
    // 在实际应用中，应该使用相机的投影矩阵和视图矩阵
    Standard_Real scale = 1.0; // 缩放因子
    Standard_Real z = 0.0;     // 固定Z坐标
    
    gp_Pnt result(normalizedX * scale * 50.0, normalizedY * scale * 50.0, z);
    return result;
}

gp_Pnt QuarterViewer::getPointFromScreen(int x, int y)
{
    return getPointFromScreen(QPoint(x, y));
}

void QuarterViewer::mousePressEvent(QMouseEvent *event)
{
    // 计算3D点并发出信号
    gp_Pnt point = getPointFromScreen(event->x(), event->y());
    emit mousePressed(point);
    
    // 保存当前按下的鼠标位置
    m_lastMousePos = event->pos();
    
    qDebug() << "QuarterViewer: Mouse pressed at screen position" << event->pos() << ", 3D point" << point.X() << point.Y() << point.Z();
    
    // 调用基类的事件处理
    QWidget::mousePressEvent(event);
    
    // QuarterWidget的processSoEvent需要SoEvent类型，这里注释掉不兼容的调用
    // 直接使用Qt的事件系统和QuarterWidget的update()方法来更新视图
    // if (m_quarterWidget) {
    //     m_quarterWidget->processSoEvent(event);
    // }
}

void QuarterViewer::mouseMoveEvent(QMouseEvent *event)
{
    // 计算3D点并发出信号
    gp_Pnt point = getPointFromScreen(event->x(), event->y());
    emit mouseMoved(point);
    
    qDebug() << "QuarterViewer: Mouse moved at screen position" << event->pos() << ", 3D point" << point.X() << point.Y() << point.Z();
    
    // 调用基类的事件处理
    QWidget::mouseMoveEvent(event);
    
    // QuarterWidget的processSoEvent需要SoEvent类型，这里注释掉不兼容的调用
    // 直接使用Qt的事件系统和QuarterWidget的update()方法来更新视图
    // if (m_quarterWidget) {
    //     m_quarterWidget->processSoEvent(event);
    // }
    m_quarterWidget->update();
}

void QuarterViewer::mouseReleaseEvent(QMouseEvent *event)
{
    // 计算3D点并发出信号
    gp_Pnt point = getPointFromScreen(event->x(), event->y());
    emit mouseReleased(point);
    
    qDebug() << "QuarterViewer: Mouse released at screen position" << event->pos() << ", 3D point" << point.X() << point.Y() << point.Z();
    
    // 调用基类的事件处理
    QWidget::mouseReleaseEvent(event);
    
    // QuarterWidget的processSoEvent需要SoEvent类型，这里注释掉不兼容的调用
    // 直接使用Qt的事件系统和QuarterWidget的update()方法来更新视图
    // if (m_quarterWidget) {
    //     m_quarterWidget->processSoEvent(event);
    // }
    m_quarterWidget->update();
}

SoNode *QuarterViewer::shapeToCoinNode(const TopoDS_Shape &shape, const Quantity_Color &color, int lineStyle, int lineWidth)
{
    SoSeparator *separator = new SoSeparator();
    separator->setName("ShapeNode");
    
    // 设置材料属性
    SoMaterial *material = new SoMaterial();
    Standard_Real r, g, b;
    r = color.Red();
    g = color.Green();
    b = color.Blue();
    material->diffuseColor.setValue(r, g, b);
    material->emissiveColor.setValue(r * 0.2, g * 0.2, b * 0.2); // 添加一些自发光效果
    separator->addChild(material);
    
    // 设置绘制样式
    SoDrawStyle *drawStyle = new SoDrawStyle();
    drawStyle->lineWidth.setValue(lineWidth);
    
    // 根据线型设置不同的线条模式
    switch (lineStyle) {
    case 0: // 实线
        drawStyle->linePattern.setValue(0xffff);
        break;
    case 1: // 虚线
        drawStyle->linePattern.setValue(0xf0f0);
        break;
    case 2: // 点线
        drawStyle->linePattern.setValue(0x8888);
        break;
    default:
        drawStyle->linePattern.setValue(0xffff);
    }
    
    separator->addChild(drawStyle);
    
    // 根据形状类型创建不同的几何节点
    try {
        if (shape.IsNull()) {
            return nullptr;
        }
        
        SoCoordinate3 *coords = new SoCoordinate3();
        SoLineSet *lineSet = new SoLineSet();
        int pointIndex = 0;
        std::vector<int> numVertices;
        
        // 统一处理：遍历形状中的所有边
        TopExp_Explorer edgeExplorer(shape, TopAbs_EDGE);
        
        while (edgeExplorer.More()) {
            TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());
            
            if (!edge.IsNull() && BRep_Tool::IsGeometric(edge)) {
                // 获取边的两个端点
                gp_Pnt start, end;
                TopExp_Explorer vertexExp(edge, TopAbs_VERTEX);
                
                if (vertexExp.More()) {
                    start = BRep_Tool::Pnt(TopoDS::Vertex(vertexExp.Current()));
                    vertexExp.Next();
                    if (vertexExp.More()) {
                        end = BRep_Tool::Pnt(TopoDS::Vertex(vertexExp.Current()));
                        
                        // 添加边的两个端点
                        coords->point.set1Value(pointIndex++, start.X(), start.Y(), start.Z());
                        coords->point.set1Value(pointIndex++, end.X(), end.Y(), end.Z());
                        numVertices.push_back(2); // 每条边有2个顶点
                    }
                }
            }
            
            edgeExplorer.Next();
        }
        
        // 如果没有边，尝试直接获取顶点（处理点类型）
        if (numVertices.empty()) {
            TopExp_Explorer vertexExp(shape, TopAbs_VERTEX);
            if (vertexExp.More()) {
                gp_Pnt p = BRep_Tool::Pnt(TopoDS::Vertex(vertexExp.Current()));
                
                // 创建一个十字线来表示点
                Standard_Real size = lineWidth * 2.0;
                coords->point.set1Value(pointIndex++, p.X() - size, p.Y(), p.Z());
                coords->point.set1Value(pointIndex++, p.X() + size, p.Y(), p.Z());
                numVertices.push_back(2); // 水平线
                
                coords->point.set1Value(pointIndex++, p.X(), p.Y() - size, p.Z());
                coords->point.set1Value(pointIndex++, p.X(), p.Y() + size, p.Z());
                numVertices.push_back(2); // 垂直线
            }
        }
        
        // 如果有顶点数据，添加到场景中
        if (!numVertices.empty()) {
            lineSet->numVertices.setValues(0, numVertices.size(), numVertices.data());
            separator->addChild(coords);
            separator->addChild(lineSet);
            qDebug() << "QuarterViewer: Added" << numVertices.size() << "edges," << pointIndex << "points";
        }
        else {
            qDebug() << "QuarterViewer: No vertices found in shape";
            // 至少添加一个可见的点，避免空节点
            coords->point.set1Value(0, 0, 0, 0);
            SoPointSet *pointSet = new SoPointSet();
            separator->addChild(coords);
            separator->addChild(pointSet);
        }
    } catch (const Standard_Failure &e) {
        qDebug() << "QuarterViewer: OCCT exception in shapeToCoinNode:" << e.GetMessageString();
        // 添加一个可见的错误标记
        SoCoordinate3 *coords = new SoCoordinate3();
        coords->point.set1Value(0, 0, 0, 0);
        SoPointSet *pointSet = new SoPointSet();
        separator->addChild(coords);
        separator->addChild(pointSet);
    }
    
    return separator;
}