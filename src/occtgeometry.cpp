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
#include <TColgp_Array1OfPnt.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>

#include <QDebug>

OCCTGeometry::OCCTGeometry(QObject *parent) : QObject(parent)
{
    m_currentType = OCCT_Point;
    m_currentColor = Quantity_Color(Quantity_NOC_BLACK);
    m_currentLineStyle = 0; // 实线
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
    // 对于曲线，持续收集点
    if (m_currentType == OCCT_Curve) {
        // 添加新点到曲线点集合
        m_curvePoints.push_back(point);
        
        // 实时预览（仅当有足够的点时）
        if (m_curvePoints.size() > 1) {
            // 计算与上一个点的距离，避免添加太近的点
            const gp_Pnt &lastPoint = m_curvePoints[m_curvePoints.size() - 2];
            double distance = point.Distance(lastPoint);
            
            // 距离阈值（可以根据需要调整）
            const double distanceThreshold = 1.0;
            
            // 只有当点距离足够远时才保留，避免点过于密集
            if (distance < distanceThreshold && m_curvePoints.size() > 2) {
                m_curvePoints.pop_back(); // 移除太近的点
            } else {
                qDebug() << "OCCTGeometry: Added point to curve, total points:" << m_curvePoints.size();
                
                // 触发geometryChanged信号，用于实时预览
                emit geometryChanged();
            }
        }
    }
}

void OCCTGeometry::finishDrawing(const gp_Pnt &point)
{
	qDebug() << "OCCTGeometry: Finishing drawing at" << point.X() << "," << point.Y() << "," << point.Z();

	// 根据当前几何类型创建相应的几何形状
	TopoDS_Shape shape;

	try {
		switch (m_currentType) {
		case OCCT_Point: {
			shape = createPoint(point);
			break;
		}
		case OCCT_Line: {
			// 确保起点和终点不重合
			if (m_startPoint.Distance(point) > 0.001) {
				shape = createLine(m_startPoint, point);
			}
			else {
				qDebug() << "OCCTGeometry: Line endpoints are too close, skipping";
				return; // 如果太近则不创建线
			}
			break;
		}
		case OCCT_Curve: {
			// 对于曲线，我们使用收集的所有点
			if (m_curvePoints.empty()) {
				m_curvePoints.push_back(m_startPoint);
			}

			// 使用收集的所有点创建曲线
			if (m_curvePoints.size() >= 1) {
				m_curvePoints.push_back(point);
				shape = createCurve(m_curvePoints);
				m_curvePoints.clear(); // 绘制完成后清空点列表
			}
			else {
				qDebug() << "OCCTGeometry: Not enough points for curve, skipping";
				m_curvePoints.clear();
				return;
			}
			break;
		}
		case OCCT_Rectangle: {
			// 确保起点和终点不重合
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
			// 计算半径（起点到终点的距离）
			double radius = m_startPoint.Distance(point);
			if (radius > 0.001) { // 确保半径有效
				shape = createCircle(m_startPoint, point);
			}
			else {
				qDebug() << "OCCTGeometry: Circle radius is too small, skipping";
				return;
			}
			break;
		}
		case OCCT_Ellipse: {
			// 确保起点和终点不重合
			if (m_startPoint.Distance(point) > 0.001) {
				// 对于椭圆，第一个点是中心，第二个点决定长轴
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

		// 如果成功创建了形状，添加到列表中
		if (!shape.IsNull()) {
			// 保存形状及其属性
			m_shapes.push_back(shape);
			m_colors.push_back(m_currentColor);
			m_lineStyles.push_back(m_currentLineStyle);
			m_lineWidths.push_back(m_currentLineWidth);
			m_geometryTypes.push_back(m_currentType);

			qDebug() << "OCCTGeometry: Shape added, total shapes:" << m_shapes.size();

			// 发出信号通知视图更新
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
    // 创建一个小立方体来表示点
    Standard_Real size = m_currentLineWidth * 0.5;
    // 使用两个对角点创建立方体，避免6参数构造函数不匹配问题
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
    
    // 创建矩形的四个顶点
    gp_Pnt p3(minX, maxY, 0);
    gp_Pnt p4(maxX, minY, 0);
    
    // 使用MakePolygon创建矩形
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
    // 计算长轴和短轴长度（这里简化处理，使用固定比例）
    gp_Vec majorAxis = center.XYZ() - point1.XYZ();
    Standard_Real majorRadius = majorAxis.Magnitude();
    
    // 短轴长度设为长轴的一半
    Standard_Real minorRadius = majorRadius * 0.5;
    
    // 创建椭圆
    GC_MakeEllipse ellipse(gp_Ax2(center, gp::DZ()), majorRadius, minorRadius);
    return BRepBuilderAPI_MakeEdge(ellipse.Value());
}