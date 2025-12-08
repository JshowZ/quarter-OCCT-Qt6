#include "OccWidget.h"
#include <AIS_Shape.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <WNT_Window.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <V3d_Viewer.hxx>
#include <AIS_InteractiveContext.hxx>

#include <QMouseEvent>

OccWidget::OccWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , m_isRotating(false)
    , m_isPanning(false)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

OccWidget::~OccWidget()
{
}

void OccWidget::initializeGL()
{
    initOCC();
}

void OccWidget::paintGL()
{
    if (!m_view.IsNull()) {
        m_view->Redraw();
    }
}

void OccWidget::resizeGL(int width, int height)
{
    if (!m_view.IsNull()) {
        m_view->MustBeResized();
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
    Handle(WNT_Window) wntWindow = new WNT_Window((Aspect_Handle)winId());
    m_view->SetWindow(wntWindow);

    if (!wntWindow->IsMapped()) {
        wntWindow->Map();
    }

    m_context = new AIS_InteractiveContext(m_viewer);
    m_context->SetDisplayMode(AIS_Shaded, true);

    m_view->SetBackgroundColor(Quantity_NOC_WHITE);
    m_view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_RED, 0.1, V3d_ZBUFFER);
}

void OccWidget::displayShape(const TopoDS_Shape& shape)
{
    if (!m_context.IsNull() && !shape.IsNull()) {
        Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
        m_context->Display(aisShape, false);
        update();
    }
}

void OccWidget::fitAll()
{
    if (!m_view.IsNull()) {
        m_view->FitAll();
        m_view->ZFitAll();
        update();
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
            m_view->Rotation(currentPos.x(), currentPos.y());
        }
        else if (m_isPanning) {
            m_view->Pan(dx, -dy);
        }

        m_lastMousePos = currentPos;
        update();
    }
}

void OccWidget::wheelEvent(QWheelEvent* event)
{
    if (!m_view.IsNull()) {
        double zoomFactor = 1.1;
        if (event->angleDelta().y() < 0) {
            zoomFactor = 1.0 / zoomFactor;
        }
        m_view->SetZoom(zoomFactor);
        update();
    }
}