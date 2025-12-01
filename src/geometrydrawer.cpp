#include "geometrydrawer.h"
#include <QPainterPath>

GeometryDrawer::GeometryDrawer(QObject *parent) : QObject(parent)
{
    m_currentType = Point;
    m_currentPen = QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}

void GeometryDrawer::setCurrentGeometryType(GeometryType type)
{
    m_currentType = type;
}

void GeometryDrawer::setLineStyle(Qt::PenStyle style)
{
    m_currentPen.setStyle(style);
}

void GeometryDrawer::setLineColor(const QColor &color)
{
    m_currentPen.setColor(color);
}

void GeometryDrawer::setLineWidth(int width)
{
    m_currentPen.setWidth(width);
}

void GeometryDrawer::startDrawing(const QPointF &point)
{
    m_startPoint = point;
    m_currentPath = QPainterPath();
    m_currentPath.moveTo(point);
}

void GeometryDrawer::continueDrawing(const QPointF &point)
{
    if (m_currentType == Curve) {
        m_currentPath.lineTo(point);
    }
}

void GeometryDrawer::finishDrawing(const QPointF &point)
{
    switch (m_currentType) {
    case Point:
        m_currentPath.addEllipse(point, m_currentPen.width() / 2, m_currentPen.width() / 2);
        break;
    case Line:
        m_currentPath.lineTo(point);
        break;
    case Curve:
        m_currentPath.lineTo(point);
        break;
    case Rectangle:
        m_currentPath.addRect(QRectF(m_startPoint, point).normalized());
        break;
    case Circle: {
        qreal radius = QLineF(m_startPoint, point).length();
        m_currentPath.addEllipse(m_startPoint, radius, radius);
        break;
    }
    case Ellipse:
        m_currentPath.addEllipse(QRectF(m_startPoint, point).normalized());
        break;
    }
    
    // Save drawn geometry and pen settings
    m_geometries.push_back(m_currentPath);
    m_geometryPens.push_back(m_currentPen);
    m_geometryTypes.push_back(m_currentType);
    
    emit geometryChanged();
}

const std::vector<QPainterPath> &GeometryDrawer::getGeometries() const
{
    return m_geometries;
}

const std::vector<QPen> &GeometryDrawer::getGeometryPens() const
{
    return m_geometryPens;
}

const std::vector<GeometryType> &GeometryDrawer::getGeometryTypes() const
{
    return m_geometryTypes;
}