#include "occtgeometry.h"
#include <BRep_Builder.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <GC_MakeSegment.hxx>
#include <GC_MakeCircle.hxx>
#include <GC_MakeEllipse.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_Ellipse.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>

#include <QDebug>

OCCTGeometry::OCCTGeometry(QObject *parent) : QObject(parent)
{
    m_currentType = OCCT_Point;
    m_currentColor = Quantity_Color(Quantity_NOC_BLACK);
    m_currentLineStyle = 0; // Solid line
    m_currentLineWidth = 2;
}

void OCCTGeometry::setCurrentGeometryType(OCCTGeometryType type)
{
    m_currentType = type;
    qDebug() << "OCCTGeometry: Current geometry type set to" << type;
}

void OCCTGeometry::setLineStyle(int style)
{
    m_currentLineStyle = style;
    qDebug() << "OCCTGeometry: Line style set to" << style;
}

void OCCTGeometry::setLineColor(const QColor &color)
{
    m_currentColor = qtToOCCTColor(color);
    
    // Explicit color conversion for debugging
    Standard_Real r = color.redF();
    Standard_Real g = color.greenF();
    Standard_Real b = color.blueF();
    
    qDebug() << "OCCTGeometry: Color set to R=" << r << "G=" << g << "B=" << b;
}

void OCCTGeometry::setLineWidth(int width)
{
    m_currentLineWidth = width;
    qDebug() << "OCCTGeometry: Line width set to" << width;
}

void OCCTGeometry::startDrawing(const gp_Pnt &startPoint)
{
    m_startPoint = startPoint;
    // For curves, clear previous points and add start point
    if (m_currentType == OCCT_Curve) {
        m_curvePoints.clear();
        m_curvePoints.push_back(startPoint);
    }
    
    qDebug() << "OCCTGeometry: Started drawing at" << startPoint.X() << "," << startPoint.Y() << "," << startPoint.Z();
}

void OCCTGeometry::continueDrawing(const gp_Pnt &point)
{
    // For curves, continuously collect points
    if (m_currentType == OCCT_Curve) {
        // Add new point to curve point collection
        m_curvePoints.push_back(point);
        
        // Real-time preview (only when there are enough points)
        if (m_curvePoints.size() > 1) {
            // Calculate distance to previous point to avoid adding too close points
            const gp_Pnt &lastPoint = m_curvePoints[m_curvePoints.size() - 2];
            double distance = point.Distance(lastPoint);
            
            // Distance threshold (can be adjusted as needed)
            const double distanceThreshold = 1.0;
            
            // Only keep points when they are far enough apart to avoid dense points
            if (distance < distanceThreshold && m_curvePoints.size() > 2) {
                m_curvePoints.pop_back(); // Remove too close points
            } else {
                qDebug() << "OCCTGeometry: Added point to curve, total points:" << m_curvePoints.size();
                
                // Trigger geometryChanged signal for real-time preview
                emit geometryChanged();
            }
        }
    }
}

void OCCTGeometry::finishDrawing(const gp_Pnt &point)
{
	qDebug() << "OCCTGeometry: Finishing drawing at" << point.X() << "," << point.Y() << "," << point.Z();

	// Create corresponding geometry shape according to current geometry type
	TopoDS_Shape shape;

	try {
		switch (m_currentType) {
		case OCCT_Point: {
			shape = createPoint(point);
			break;
		}
		case OCCT_Line: {
			// Ensure start and end points are not coincident
			if (m_startPoint.Distance(point) > 0.001) {
				shape = createLine(m_startPoint, point);
			}
			else {
				qDebug() << "OCCTGeometry: Line endpoints are too close, skipping";
				return; // Don't create line if too close
			}
			break;
		}
		case OCCT_Curve: {
			// For curves, we use all collected points
			if (m_curvePoints.empty()) {
				m_curvePoints.push_back(m_startPoint);
			}

			// Create curve using all collected points
			if (m_curvePoints.size() >= 1) {
				m_curvePoints.push_back(point);
				shape = createCurve(m_curvePoints);
				m_curvePoints.clear(); // Clear point list after drawing
			}
			else {
				qDebug() << "OCCTGeometry: Not enough points for curve, skipping";
				m_curvePoints.clear();
				return;
			}
			break;
		}
		case OCCT_Rectangle: {
			// Ensure start and end points are not coincident
			if (m_startPoint.Distance(point) > 0.001) {
				shape = createRectangle(m_startPoint, point);
			}
			else {
				qDebug() << "OCCTGeometry: Rectangle corners are too close, skipping";
				return;
			}
			break;
		}
		case OCCT_Circle: {
			// Calculate radius (distance from start to end point)
			double radius = m_startPoint.Distance(point);
			if (radius > 0.001) { // Ensure radius is valid
				shape = createCircle(m_startPoint, point);
			}
			else {
				qDebug() << "OCCTGeometry: Circle radius is too small, skipping";
				return;
			}
			break;
		}
		case OCCT_Ellipse: {
			// Ensure start and end points are not coincident
			if (m_startPoint.Distance(point) > 0.001) {
				// For ellipse, first point is center, second point determines major axis
				shape = createEllipse(m_startPoint, point);
			}
			else {
				qDebug() << "OCCTGeometry: Ellipse points are too close, skipping";
				return;
			}
			break;
		}
		default: {
			qDebug() << "OCCTGeometry: Unknown geometry type";
			break;
		}
		}

		// If shape was successfully created, add to list
		if (!shape.IsNull()) {
			// Save shape and its properties
			m_shapes.push_back(shape);
			m_colors.push_back(m_currentColor);
			m_lineStyles.push_back(m_currentLineStyle);
			m_lineWidths.push_back(m_currentLineWidth);
			m_geometryTypes.push_back(m_currentType);

			qDebug() << "OCCTGeometry: Shape added, total shapes:" << m_shapes.size();

			// Emit signal to notify view update
			emit geometryChanged();
		}
		else {
			qDebug() << "OCCTGeometry: Failed to create shape";
		}
	}
	catch (const Standard_Failure& e) {
		qDebug() << "OCCTGeometry: OCCT exception during shape creation:" << e.GetMessageString();
	}
	catch (...) {
		qDebug() << "OCCTGeometry: Unknown exception during shape creation";
	}
}

const std::vector<TopoDS_Shape> &OCCTGeometry::getShapes() const
{
    return m_shapes;
}

const std::vector<Quantity_Color> &OCCTGeometry::getColors() const
{
    return m_colors;
}

const std::vector<int> &OCCTGeometry::getLineStyles() const
{
    return m_lineStyles;
}

const std::vector<int> &OCCTGeometry::getLineWidths() const
{
    return m_lineWidths;
}

const std::vector<OCCTGeometryType> &OCCTGeometry::getGeometryTypes() const
{
    return m_geometryTypes;
}

void OCCTGeometry::clearAll()
{
    m_shapes.clear();
    m_colors.clear();
    m_lineStyles.clear();
    m_lineWidths.clear();
    m_geometryTypes.clear();
    
    emit geometryChanged();
}

QPointF OCCTGeometry::occtToQtPoint(const gp_Pnt &point)
{
    return QPointF(point.X(), point.Y());
}

gp_Pnt OCCTGeometry::qtToOCCTPoint(const QPointF &point)
{
    return gp_Pnt(point.x(), point.y(), 0);
}

Quantity_Color OCCTGeometry::qtToOCCTColor(const QColor &color)
{
    return Quantity_Color(color.redF(), color.greenF(), color.blueF(), Quantity_TOC_RGB);
}

QColor OCCTGeometry::occtToQtColor(const Quantity_Color &color)
{
    Standard_Real r, g, b;
    color.Values(r, g, b, Quantity_TOC_RGB);
    return QColor::fromRgbF(r, g, b);
}

TopoDS_Shape OCCTGeometry::createPoint(const gp_Pnt &point)
{
    // Create a small cube to represent a point
    Standard_Real size = m_currentLineWidth * 0.5;
    // Create cube using two diagonal points to avoid 6-parameter constructor mismatch
    gp_Pnt corner1(point.X() - size, point.Y() - size, point.Z() - size);
    gp_Pnt corner2(point.X() + size, point.Y() + size, point.Z() + size);
    BRepPrimAPI_MakeBox box(corner1, corner2);
    return box.Shape();
}

TopoDS_Shape OCCTGeometry::createLine(const gp_Pnt &start, const gp_Pnt &end)
{
    GC_MakeSegment segment(start, end);
    return BRepBuilderAPI_MakeEdge(segment.Value());
}

TopoDS_Shape OCCTGeometry::createCurve(const std::vector<gp_Pnt> &points)
{
    if (points.size() < 2) {
        return TopoDS_Shape();
    }
    
    BRepBuilderAPI_MakeWire wire;
    for (size_t i = 1; i < points.size(); ++i) {
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(points[i-1], points[i]);
        wire.Add(edge);
    }
    return wire.Wire();
}

TopoDS_Shape OCCTGeometry::createRectangle(const gp_Pnt &p1, const gp_Pnt &p2)
{
    Standard_Real minX = std::min(p1.X(), p2.X());
    Standard_Real minY = std::min(p1.Y(), p2.Y());
    Standard_Real maxX = std::max(p1.X(), p2.X());
    Standard_Real maxY = std::max(p1.Y(), p2.Y());
    
    // Create four vertices of the rectangle
    gp_Pnt p3(minX, maxY, 0);
    gp_Pnt p4(maxX, minY, 0);
    
    // Create rectangle using MakePolygon
    BRepBuilderAPI_MakePolygon polygon;
    polygon.Add(p1);
    polygon.Add(p3);
    polygon.Add(p2);
    polygon.Add(p4);
    polygon.Close();
    
    return polygon.Wire();
}

TopoDS_Shape OCCTGeometry::createCircle(const gp_Pnt &center, const gp_Pnt &edge)
{
    Standard_Real radius = center.Distance(edge);
    GC_MakeCircle circle(center, gp::DZ(), radius);
    return BRepBuilderAPI_MakeEdge(circle.Value());
}

TopoDS_Shape OCCTGeometry::createEllipse(const gp_Pnt &center, const gp_Pnt &point1)
{
    // Calculate major and minor axis lengths (simplified, using fixed ratio)
    gp_Vec majorAxis = center.XYZ() - point1.XYZ();
    Standard_Real majorRadius = majorAxis.Magnitude();
    
    // Set minor axis length to half of major axis
    Standard_Real minorRadius = majorRadius * 0.5;
    
    // Create ellipse
    GC_MakeEllipse ellipse(gp_Ax2(center, gp::DZ()), majorRadius, minorRadius);
    return BRepBuilderAPI_MakeEdge(ellipse.Value());
}