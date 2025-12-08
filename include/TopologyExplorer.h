#ifndef TOPOLOGYEXPLORER_H
#define TOPOLOGYEXPLORER_H

#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopAbs.hxx>
#include <Geom_Curve.hxx>
#include <Geom2d_Curve.hxx>
#include <TColStd_IndexedMapOfTransient.hxx>
#include <TopLoc_Location.hxx>
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <Geom_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomConvert.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom2d_Ellipse.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_BezierCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <TCollection_ExtendedString.hxx>
#include <Message_ProgressIndicator.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <NCollection_Map.hxx>

#include <vector>
#include <memory>

/**
 * @class TopologyExplorer
 * @brief 用于遍历OCCT拓扑结构并提取所有唯一几何曲线的工具类
 */
class TopologyExplorer
{
public:
    // 构造函数
    TopologyExplorer();
    
    // 设置进度指示器（可选）
    void SetProgressIndicator(const Handle(Message_ProgressIndicator)& progress);
    
    // 主遍历函数
    bool Explore(const TopoDS_Shape& shape);
    
    // 获取特定类型的拓扑形状
    const TopTools_IndexedMapOfShape& GetShapesByType(TopAbs_ShapeEnum type) const;
    
    // 获取所有唯一的3D曲线
    const TColStd_IndexedMapOfTransient& GetUniqueCurves3D() const;
    
    // 获取所有唯一的2D曲线（从面上提取）
    const TColStd_IndexedMapOfTransient& GetUniqueCurves2D() const;
    
    // 获取曲线类型统计信息
    void GetCurveStatistics(TCollection_ExtendedString& stats) const;
    
    // 获取拓扑类型统计信息
    void GetTopologyStatistics(TCollection_ExtendedString& stats) const;
    
    // 清除所有数据
    void Clear();
    
    // 导出所有曲线到STEP文件（仅3D曲线）
    bool ExportCurvesToSTEP(const TCollection_ExtendedString& filename) const;
    
    // 查找特定类型的曲线（例如所有B样条曲线）
    std::vector<Handle(Geom_Curve)> FindCurvesByType(Standard_CString typeName) const;
    
    // 调试：打印所有信息
    void PrintInfo() const;

private:
    // 内部遍历实现
    void ExploreRecursive(const TopoDS_Shape& shape, const TopLoc_Location& location);
    
    // 处理边并提取曲线
    void ProcessEdge(const TopoDS_Edge& edge, const TopLoc_Location& location);
    
    // 处理面并提取2D曲线
    void ProcessFace(const TopoDS_Face& face, const TopLoc_Location& location);
    
    // 添加3D曲线到唯一集合
    void AddUniqueCurve3D(const Handle(Geom_Curve)& curve);
    
    // 添加2D曲线到唯一集合
    void AddUniqueCurve2D(const Handle(Geom2d_Curve)& curve);
    
    // 获取曲线类型名称
    TCollection_ExtendedString GetCurveTypeName(const Handle(Geom_Curve)& curve) const;
    
    // 获取2D曲线类型名称
    TCollection_ExtendedString GetCurve2DTypeName(const Handle(Geom2d_Curve)& curve) const;

private:
    // 按类型存储的拓扑形状
    TopTools_IndexedMapOfShape m_shapeMaps[TopAbs_SHAPE];
    
    // 唯一的3D曲线集合
    TColStd_IndexedMapOfTransient m_uniqueCurves3D;
    
    // 唯一的2D曲线集合
    TColStd_IndexedMapOfTransient m_uniqueCurves2D;
    
    // 进度指示器
    Handle(Message_ProgressIndicator) m_progress;
    
    // 统计信息
    struct CurveStats {
        int lineCount = 0;
        int circleCount = 0;
        int ellipseCount = 0;
        int bsplineCount = 0;
        int bezierCount = 0;
        int trimmedCount = 0;
        int otherCount = 0;
    } m_curveStats3D, m_curveStats2D;
};

#endif // TOPOLOGYEXPLORER_H