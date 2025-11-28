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

// OCCT几何图形类型枚举
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
    
    // 设置当前绘制类型
    void setCurrentGeometryType(OCCTGeometryType type);
    
    // 设置线型
    void setLineStyle(int style);
    
    // 设置颜色
    void setLineColor(const QColor &color);
    
    // 设置线宽
    void setLineWidth(int width);
    
    // 开始绘制（鼠标按下）
    void startDrawing(const gp_Pnt &point);
    
    // 继续绘制（鼠标移动）
    void continueDrawing(const gp_Pnt &point);
    
    // 结束绘制（鼠标释放）
    void finishDrawing(const gp_Pnt &point);
    
    // 获取当前所有图形
    const std::vector<TopoDS_Shape> &getShapes() const;
    const std::vector<Quantity_Color> &getColors() const;
    const std::vector<int> &getLineStyles() const;
    const std::vector<int> &getLineWidths() const;
    const std::vector<OCCTGeometryType> &getGeometryTypes() const;
    
    // 清除所有图形
    void clearAll();
    
    // 从OCCT点转换为Qt点
    static QPointF occtToQtPoint(const gp_Pnt &point);
    
    // 从Qt点转换为OCCT点
    static gp_Pnt qtToOCCTPoint(const QPointF &point);
    
    // 从QColor转换为Quantity_Color
    static Quantity_Color qtToOCCTColor(const QColor &color);
    
    // 从Quantity_Color转换为QColor
    static QColor occtToQtColor(const Quantity_Color &color);

signals:
    // 当需要更新视图时发出信号
    void geometryChanged();
    
private:
    OCCTGeometryType m_currentType;
    Quantity_Color m_currentColor;
    int m_currentLineStyle;
    int m_currentLineWidth;
    gp_Pnt m_startPoint;
    
    // 用于曲线绘制的点集合
    std::vector<gp_Pnt> m_curvePoints;
    
    std::vector<TopoDS_Shape> m_shapes;
    std::vector<Quantity_Color> m_colors;
    std::vector<int> m_lineStyles;
    std::vector<int> m_lineWidths;
    std::vector<OCCTGeometryType> m_geometryTypes;
    
    // 创建点
    TopoDS_Shape createPoint(const gp_Pnt &point);
    
    // 创建直线
    TopoDS_Shape createLine(const gp_Pnt &start, const gp_Pnt &end);
    
    // 创建曲线
    TopoDS_Shape createCurve(const std::vector<gp_Pnt> &points);
    
    // 创建矩形
    TopoDS_Shape createRectangle(const gp_Pnt &p1, const gp_Pnt &p2);
    
    // 创建圆形
    TopoDS_Shape createCircle(const gp_Pnt &center, const gp_Pnt &edge);
    
    // 创建椭圆
    TopoDS_Shape createEllipse(const gp_Pnt &center, const gp_Pnt &point1);
};

#endif // OCCTGEOMETRY_H