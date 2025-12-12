#include "OccWidget.h"
#include <AIS_Shape.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <WNT_Window.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <V3d_Viewer.hxx>
#include <AIS_InteractiveContext.hxx>

#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QFocusEvent>

OccWidget::OccWidget(QWidget* parent)
    : QWidget(parent)
    , m_isRotating(false)
    , m_isPanning(false)
    , m_isInitialized(false)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAttribute(Qt::WA_NativeWindow, true); // 确保窗口有原生句柄
    setAttribute(Qt::WA_PaintOnScreen, true); // 直接在屏幕上绘制
}

OccWidget::~OccWidget()
{
    // 确保正确释放资源
    if (!m_wntWindow.IsNull()) {
        m_wntWindow->Unmap();
    }
}

void OccWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    
    // 在窗口显示后初始化OCC，确保有有效的窗口句柄
    if (!m_isInitialized) {
        initOCC();
        m_isInitialized = true;
    }
}

void OccWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    // 确保OCC已初始化
    if (!m_isInitialized) {
        initOCC();
        m_isInitialized = true;
    }
    
    if (!m_view.IsNull()) {
        m_view->Redraw();
    }
}

void OccWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    
    if (m_isInitialized && !m_view.IsNull()) {
        // 通知视图窗口大小已更改，使用MustBeResized()方法
        m_view->MustBeResized();
        m_view->Redraw();
    }
}

void OccWidget::focusInEvent(QFocusEvent* event)
{
    QWidget::focusInEvent(event);
    
    // 窗口获得焦点时重绘
    if (m_isInitialized && !m_view.IsNull()) {
        m_view->Redraw();
    }
}

void OccWidget::focusOutEvent(QFocusEvent* event)
{
    QWidget::focusOutEvent(event);
    
    // 窗口失去焦点时重绘（可选，根据需要调整）
    if (m_isInitialized && !m_view.IsNull()) {
        m_view->Redraw();
    }
}

void OccWidget::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    
    // 处理窗口激活状态变化
    if (event->type() == QEvent::ActivationChange) {
        if (m_isInitialized && !m_view.IsNull()) {
            m_view->Redraw();
        }
    }
}

void OccWidget::initOCC()
{
    static Handle(Aspect_DisplayConnection) displayConnection;
    if (displayConnection.IsNull()) {
        displayConnection = new Aspect_DisplayConnection();
    }

    static Handle(Graphic3d_GraphicDriver) graphicDriver;
    if (graphicDriver.IsNull()) {
        graphicDriver = new OpenGl_GraphicDriver(displayConnection);
    }

    m_viewer = new V3d_Viewer(graphicDriver);
    m_viewer->SetDefaultLights();
    m_viewer->SetLightOn();
    m_viewer->SetDefaultBackgroundColor(Quantity_NOC_WHITE);

    m_view = m_viewer->CreateView();
    
    // 创建WNT_Window并保存到成员变量
    m_wntWindow = new WNT_Window((Aspect_Handle)winId());
    m_view->SetWindow(m_wntWindow);

    if (!m_wntWindow->IsMapped()) {
        m_wntWindow->Map();
    }

    m_context = new AIS_InteractiveContext(m_viewer);
    m_context->SetDisplayMode(AIS_Shaded, true);

    m_view->SetBackgroundColor(Quantity_NOC_WHITE);
    m_view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_RED, 0.1, V3d_ZBUFFER);
    m_view->MustBeResized();
    m_view->Redraw();
}

void OccWidget::displayShape(const TopoDS_Shape& shape)
{
    if (!m_isInitialized) {
        initOCC();
        m_isInitialized = true;
    }
    
    if (!m_context.IsNull() && !shape.IsNull()) {
        Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
        m_context->Display(aisShape, false);
        update();
    }
}

void OccWidget::fitAll()
{
    if (!m_isInitialized) {
        initOCC();
        m_isInitialized = true;
    }
    
    if (!m_view.IsNull()) {
        m_view->FitAll();
        m_view->ZFitAll();
        m_view->Redraw();
    }
}

void OccWidget::eraseAll()
{
    if (!m_context.IsNull()) {
        m_context->RemoveAll(false);
        update();
    }
}

void OccWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isRotating = true;
        // 开始旋转，调用StartRotation初始化旋转状态
        m_view->StartRotation(event->pos().x(), event->pos().y());
    }
    else if (event->button() == Qt::RightButton) {
        m_isPanning = true;
    }
    m_lastMousePos = event->pos();
}

void OccWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isRotating = false;
    }
    else if (event->button() == Qt::RightButton) {
        m_isPanning = false;
    }
}

void OccWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_view.IsNull()) {
        QPoint currentPos = event->pos();
        int dx = currentPos.x() - m_lastMousePos.x();
        int dy = currentPos.y() - m_lastMousePos.y();

        if (m_isRotating) {
            // 继续旋转，传递当前鼠标位置
            m_view->Rotation(currentPos.x(), currentPos.y());
        }
        else if (m_isPanning) {
            // 使用正确的平移方法，OCCT的Y轴与Qt相反
            m_view->Pan(dx, -dy);
        }

        m_lastMousePos = currentPos;
        m_view->Redraw();
    }
}

void OccWidget::wheelEvent(QWheelEvent* event)
{
    if (!m_view.IsNull()) {
        double zoomFactor = 1.0;
        
        // 计算缩放因子
        if (event->angleDelta().y() > 0) {
            zoomFactor = 1.1; // 放大
        } else {
            zoomFactor = 0.9; // 缩小
        }
        
        // 使用正确的缩放方法，SetZoom接受缩放因子和可选的布尔参数
        m_view->SetZoom(zoomFactor, false); // false表示相对于当前状态缩放
        m_view->Redraw();
    }
}