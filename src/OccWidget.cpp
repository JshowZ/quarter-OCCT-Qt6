#include "OccWidget.h"
#include <AIS_Shape.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <WNT_Window.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <V3d_Viewer.hxx>
#include <AIS_InteractiveContext.hxx>

#include <QMouseEvent>
#include <QShowEvent>
#include <QResizeEvent>

OccWidget::OccWidget(QWidget* parent)
    : QWidget(parent)
    , m_isInitialized(false)
    , m_isRotating(false)
    , m_isPanning(false)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
}

OccWidget::~OccWidget()
{
    if (!m_wntWindow.IsNull() && m_wntWindow->IsMapped()) {
        m_wntWindow->Unmap();
    }
}

void OccWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    
    if (!m_isInitialized) {
        initOCC();
        m_isInitialized = true;
    }
    
    updateView();
}

void OccWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    
    if (m_isInitialized) {
        if (!m_view.IsNull()) {
            // 确保OCCT视图知道窗口大小已经变化
            m_view->MustBeResized();
            
            // 强制重绘
            m_view->Invalidate();
            m_view->Redraw();
        }
    }
}

void OccWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    
    if (m_isInitialized && !m_view.IsNull()) {
        // 在paintEvent中直接调用Redraw，确保模型始终可见
        m_view->Redraw();
    }
}

void OccWidget::initOCC()
{
    try {
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
        m_wntWindow = new WNT_Window((Aspect_Handle)winId());
        m_view->SetWindow(m_wntWindow);

        if (!m_wntWindow->IsMapped()) {
            m_wntWindow->Map();
        }

        m_context = new AIS_InteractiveContext(m_viewer);
        m_context->SetDisplayMode(AIS_Shaded, true);

        m_view->SetBackgroundColor(Quantity_NOC_WHITE);
        m_view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_RED, 0.1, V3d_ZBUFFER);
        
        updateView();
    }
    catch (const Standard_Failure& e) {
        // Handle OCCT exceptions
    }
    catch (...) {
        // Handle other exceptions
    }
}

void OccWidget::updateView()
{
    if (!m_view.IsNull()) {
        m_view->Redraw();
    }
}

void OccWidget::displayShape(const TopoDS_Shape& shape)
{
    if (m_isInitialized && !m_context.IsNull() && !shape.IsNull()) {
        Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
        m_context->Display(aisShape, false);
        updateView();
    }
}

void OccWidget::fitAll()
{
    if (m_isInitialized && !m_view.IsNull()) {
        m_view->FitAll();
        m_view->ZFitAll();
        updateView();
    }
}

void OccWidget::eraseAll()
{
    if (m_isInitialized && !m_context.IsNull()) {
        m_context->RemoveAll(false);
        updateView();
    }
}

void OccWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isRotating = true;
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
    if (m_isInitialized && !m_view.IsNull()) {
        QPoint currentPos = event->pos();
        int dx = currentPos.x() - m_lastMousePos.x();
        int dy = currentPos.y() - m_lastMousePos.y();

        if (m_isRotating) {
            m_view->Rotation(currentPos.x(), currentPos.y());
            updateView();
        }
        else if (m_isPanning) {
            m_view->Pan(dx, -dy);
            updateView();
        }

        m_lastMousePos = currentPos;
    }
}

void OccWidget::wheelEvent(QWheelEvent* event)
{
    if (m_isInitialized && !m_view.IsNull()) {
        double zoomFactor = 1.1;
        if (event->angleDelta().y() < 0) {
            zoomFactor = 1.0 / zoomFactor;
        }
        m_view->SetZoom(zoomFactor);
        updateView();
    }
}