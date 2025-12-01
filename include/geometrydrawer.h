#ifndef GEOMETRYDRAWER_H
#define GEOMETRYDRAWER_H

#include <QObject>
#include <QColor>
#include <QPen>
#include <QPointF>
#include <QPainterPath>
#include <vector>

// Geometry type enumeration
enum GeometryType {
    Point,
    Line,
    Curve,
    Rectangle,
    Circle,
    Ellipse
};

class GeometryDrawer : public QObject
{
    Q_OBJECT
public:
    explicit GeometryDrawer(QObject *parent = nullptr);

    // Set current geometry type
    void setCurrentGeometryType(GeometryType type);
    
    // Set line style
    void setLineStyle(Qt::PenStyle style);
    
    // Set line color
    void setLineColor(const QColor &color);
    
    // Set line width
    void setLineWidth(int width);
    
    // Start drawing (mouse press)
    void startDrawing(const QPointF &point);
    
    // Continue drawing (mouse move)
    void continueDrawing(const QPointF &point);
    
    // Finish drawing (mouse release)
    void finishDrawing(const QPointF &point);
    
    // Get all current geometry data
    const std::vector<QPainterPath> &getGeometries() const;
    const std::vector<QPen> &getGeometryPens() const;
    const std::vector<GeometryType> &getGeometryTypes() const;

signals:
    // Signal emitted when view needs to be updated
    void geometryChanged();

private:
    GeometryType m_currentType;
    QPen m_currentPen;
    QPainterPath m_currentPath;
    QPointF m_startPoint;
    
    std::vector<QPainterPath> m_geometries;
    std::vector<QPen> m_geometryPens;
    std::vector<GeometryType> m_geometryTypes;
};

#endif // GEOMETRYDRAWER_H