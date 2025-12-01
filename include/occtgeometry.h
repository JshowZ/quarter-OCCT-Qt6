#ifndef OCCTGEOMETRY_H
#define OCCTGEOMETRY_H

#include <QObject>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Vec.hxx>
#include <Quantity_Color.hxx>
#include <QColor>
#include <QPointF>

// OCCT geometry type enumeration
enum OCCTGeometryType {
    OCCT_Point,
    OCCT_Line,
    OCCT_Curve,
    OCCT_Rectangle,
    OCCT_Circle,
    OCCT_Ellipse
};

class OCCTGeometry : public QObject
{
    Q_OBJECT
public:
    explicit OCCTGeometry(QObject *parent = nullptr);
    
    // Set current geometry type
    void setCurrentGeometryType(OCCTGeometryType type);
    
    // Set line style
    void setLineStyle(int style);
    
    // Set line color
    void setLineColor(const QColor &color);
    
    // Set line width
    void setLineWidth(int width);
    
    // Start drawing (mouse press)
    void startDrawing(const gp_Pnt &point);
    
    // Continue drawing (mouse move)
    void continueDrawing(const gp_Pnt &point);
    
    // Finish drawing (mouse release)
    void finishDrawing(const gp_Pnt &point);
    
    // Get all current shapes
    const std::vector<TopoDS_Shape> &getShapes() const;
    const std::vector<Quantity_Color> &getColors() const;
    const std::vector<int> &getLineStyles() const;
    const std::vector<int> &getLineWidths() const;
    const std::vector<OCCTGeometryType> &getGeometryTypes() const;
    
    // Clear all shapes
    void clearAll();
    
    // Convert OCCT point to Qt point
    static QPointF occtToQtPoint(const gp_Pnt &point);
    
    // Convert Qt point to OCCT point
    static gp_Pnt qtToOCCTPoint(const QPointF &point);
    
    // Convert QColor to Quantity_Color
    static Quantity_Color qtToOCCTColor(const QColor &color);
    
    // Convert Quantity_Color to QColor
    static QColor occtToQtColor(const Quantity_Color &color);

signals:
    // Signal emitted when view needs to be updated
    void geometryChanged();
    
private:
    OCCTGeometryType m_currentType;
    Quantity_Color m_currentColor;
    int m_currentLineStyle;
    int m_currentLineWidth;
    gp_Pnt m_startPoint;
    
    // Point collection for curve drawing
    std::vector<gp_Pnt> m_curvePoints;
    
    std::vector<TopoDS_Shape> m_shapes;
    std::vector<Quantity_Color> m_colors;
    std::vector<int> m_lineStyles;
    std::vector<int> m_lineWidths;
    std::vector<OCCTGeometryType> m_geometryTypes;
    
    // Create point
    TopoDS_Shape createPoint(const gp_Pnt &point);
    
    // Create line
    TopoDS_Shape createLine(const gp_Pnt &start, const gp_Pnt &end);
    
    // Create curve
    TopoDS_Shape createCurve(const std::vector<gp_Pnt> &points);
    
    // Create rectangle
    TopoDS_Shape createRectangle(const gp_Pnt &p1, const gp_Pnt &p2);
    
    // Create circle
    TopoDS_Shape createCircle(const gp_Pnt &center, const gp_Pnt &edge);
    
    // Create ellipse
    TopoDS_Shape createEllipse(const gp_Pnt &center, const gp_Pnt &point1);
};

#endif // OCCTGEOMETRY_H