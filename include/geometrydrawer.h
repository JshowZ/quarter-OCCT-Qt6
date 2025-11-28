#ifndef GEOMETRYDRAWER_H
#define GEOMETRYDRAWER_H

#include <QObject>
#include <QColor>
#include <QPen>
#include <QPointF>
#include <QPainterPath>
#include <vector>

// 几何图形类型枚举
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

    // 设置当前绘制类型
    void setCurrentGeometryType(GeometryType type);
    
    // 设置线型
    void setLineStyle(Qt::PenStyle style);
    
    // 设置颜色
    void setLineColor(const QColor &color);
    
    // 设置线宽
    void setLineWidth(int width);
    
    // 开始绘制（鼠标按下）
    void startDrawing(const QPointF &point);
    
    // 继续绘制（鼠标移动）
    void continueDrawing(const QPointF &point);
    
    // 结束绘制（鼠标释放）
    void finishDrawing(const QPointF &point);
    
    // 获取当前所有图形数据
    const std::vector<QPainterPath> &getGeometries() const;
    const std::vector<QPen> &getGeometryPens() const;
    const std::vector<GeometryType> &getGeometryTypes() const;

signals:
    // 当需要更新视图时发出信号
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