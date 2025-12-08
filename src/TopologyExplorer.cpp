#include "TopologyExplorer.h"
#include <TopExp_Explorer.hxx>
#include <TopoDS_Iterator.hxx>
#include <BRepTools.hxx>
#include <STEPControl_Writer.hxx>
#include <Interface_Static.hxx>
#include <TColStd_HSequenceOfTransient.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GCPnts_UniformDeflection.hxx>
#include <TopoDS.hxx>
#include <iostream>

// 构造函数
TopologyExplorer::TopologyExplorer()
{
    // 初始化形状映射
    for (int i = 0; i < TopAbs_SHAPE; i++) {
        m_shapeMaps[i].Clear();
    }
}

// 设置进度指示器
void TopologyExplorer::SetProgressIndicator(const Handle(Message_ProgressIndicator)& progress)
{
    m_progress = progress;
}

// 主遍历函数
bool TopologyExplorer::Explore(const TopoDS_Shape& shape)
{
    Clear(); // 清除旧数据
    
    if (shape.IsNull()) {
        return false;
    }
    
    // 使用TopExp::MapShapes快速收集所有拓扑类型
    for (int type = TopAbs_COMPOUND; type < TopAbs_SHAPE; type++) {
        TopExp::MapShapes(shape, (TopAbs_ShapeEnum)type, m_shapeMaps[type]);
    }
    
    // 递归处理所有边和面以提取曲线
    TopLoc_Location identityLoc;
    ExploreRecursive(shape, identityLoc);
    
    return true;
}

// 递归遍历实现
void TopologyExplorer::ExploreRecursive(const TopoDS_Shape& shape, const TopLoc_Location& location)
{
    if (shape.IsNull()) return;
    
    TopAbs_ShapeEnum shapeType = shape.ShapeType();
    
    // 处理边
    if (shapeType == TopAbs_EDGE) {
        ProcessEdge(TopoDS::Edge(shape), location);
    }
    // 处理面
    else if (shapeType == TopAbs_FACE) {
        ProcessFace(TopoDS::Face(shape), location);
    }
    // 递归处理复合形状
    else if (shapeType == TopAbs_COMPOUND || shapeType == TopAbs_COMPSOLID || 
             shapeType == TopAbs_SOLID || shapeType == TopAbs_SHELL) {
        for (TopoDS_Iterator it(shape); it.More(); it.Next()) {
            ExploreRecursive(it.Value(), location);
        }
    }
}

// 处理边并提取曲线
void TopologyExplorer::ProcessEdge(const TopoDS_Edge& edge, const TopLoc_Location& location)
{
    Standard_Real first, last;
    // 修正BRep_Tool::Curve调用，移除location参数
    Handle(Geom_Curve) curve3d = BRep_Tool::Curve(edge, first, last);
    
    if (!curve3d.IsNull()) {
        // 如果是裁剪曲线，获取基础曲线
        Handle(Geom_TrimmedCurve) trimmed = Handle(Geom_TrimmedCurve)::DownCast(curve3d);
        if (!trimmed.IsNull()) {
            curve3d = trimmed->BasisCurve();
            m_curveStats3D.trimmedCount++;
        }
        
        // 添加到唯一集合
        AddUniqueCurve3D(curve3d);
    }
    
    // 注意：退化边（Degenerated Edge）没有独立的3D曲线
    // 它们的几何信息在面上，会在ProcessFace中处理
}

// 处理面并提取2D曲线
void TopologyExplorer::ProcessFace(const TopoDS_Face& face, const TopLoc_Location& location)
{
    TopExp_Explorer edgeExp(face, TopAbs_EDGE);
    
    for (; edgeExp.More(); edgeExp.Next()) {
        const TopoDS_Edge& edge = TopoDS::Edge(edgeExp.Current());
        
        // 获取边在面参数空间中的2D曲线
        Standard_Real first, last;
        Handle(Geom2d_Curve) curve2d = BRep_Tool::CurveOnSurface(edge, face, first, last);
        
        if (!curve2d.IsNull()) {
            // 如果是裁剪曲线，获取基础曲线
            Handle(Geom2d_TrimmedCurve) trimmed = Handle(Geom2d_TrimmedCurve)::DownCast(curve2d);
            if (!trimmed.IsNull()) {
                curve2d = trimmed->BasisCurve();
                m_curveStats2D.trimmedCount++;
            }
            
            // 添加到唯一集合
            AddUniqueCurve2D(curve2d);
        }
    }
}

// 添加3D曲线到唯一集合
void TopologyExplorer::AddUniqueCurve3D(const Handle(Geom_Curve)& curve)
{
    if (curve.IsNull()) return;
    
    // TColStd_IndexedMapOfTransient::Add方法自动去重
    // 如果元素已存在，返回现有索引；否则添加并返回新索引
    int index = m_uniqueCurves3D.Add(curve);
    
    // 只有当曲线是新添加的（索引等于当前大小）才更新统计
    if (index == m_uniqueCurves3D.Extent()) {
        // 更新统计
        if (curve->DynamicType() == STANDARD_TYPE(Geom_Line)) {
            m_curveStats3D.lineCount++;
        }
        else if (curve->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
            m_curveStats3D.circleCount++;
        }
        else if (curve->DynamicType() == STANDARD_TYPE(Geom_Ellipse)) {
            m_curveStats3D.ellipseCount++;
        }
        else if (curve->DynamicType() == STANDARD_TYPE(Geom_BSplineCurve)) {
            m_curveStats3D.bsplineCount++;
        }
        else if (curve->DynamicType() == STANDARD_TYPE(Geom_BezierCurve)) {
            m_curveStats3D.bezierCount++;
        }
        else {
            m_curveStats3D.otherCount++;
        }
    }
}

// 添加2D曲线到唯一集合
void TopologyExplorer::AddUniqueCurve2D(const Handle(Geom2d_Curve)& curve)
{
    if (curve.IsNull()) return;
    
    // TColStd_IndexedMapOfTransient::Add方法自动去重
    // 如果元素已存在，返回现有索引；否则添加并返回新索引
    int index = m_uniqueCurves2D.Add(curve);
    
    // 只有当曲线是新添加的（索引等于当前大小）才更新统计
    if (index == m_uniqueCurves2D.Extent()) {
        // 更新统计
        if (curve->DynamicType() == STANDARD_TYPE(Geom2d_Line)) {
            m_curveStats2D.lineCount++;
        }
        else if (curve->DynamicType() == STANDARD_TYPE(Geom2d_Circle)) {
            m_curveStats2D.circleCount++;
        }
        else if (curve->DynamicType() == STANDARD_TYPE(Geom2d_Ellipse)) {
            m_curveStats2D.ellipseCount++;
        }
        else if (curve->DynamicType() == STANDARD_TYPE(Geom2d_BSplineCurve)) {
            m_curveStats2D.bsplineCount++;
        }
        else if (curve->DynamicType() == STANDARD_TYPE(Geom2d_BezierCurve)) {
            m_curveStats2D.bezierCount++;
        }
        else {
            m_curveStats2D.otherCount++;
        }
    }
}

// 获取特定类型的拓扑形状
const TopTools_IndexedMapOfShape& TopologyExplorer::GetShapesByType(TopAbs_ShapeEnum type) const
{
    if (type >= 0 && type < TopAbs_SHAPE) {
        return m_shapeMaps[type];
    }
    static TopTools_IndexedMapOfShape emptyMap;
    return emptyMap;
}

// 获取所有唯一的3D曲线
const TColStd_IndexedMapOfTransient& TopologyExplorer::GetUniqueCurves3D() const
{
    return m_uniqueCurves3D;
}

// 获取所有唯一的2D曲线
const TColStd_IndexedMapOfTransient& TopologyExplorer::GetUniqueCurves2D() const
{
    return m_uniqueCurves2D;
}

// 获取曲线类型统计信息
void TopologyExplorer::GetCurveStatistics(TCollection_ExtendedString& stats) const
{
    stats.Clear();
    stats += "3D Curves Statistics:\n";
    stats += TCollection_ExtendedString("  Lines: ") + m_curveStats3D.lineCount + "\n";
    stats += TCollection_ExtendedString("  Circles: ") + m_curveStats3D.circleCount + "\n";
    stats += TCollection_ExtendedString("  Ellipses: ") + m_curveStats3D.ellipseCount + "\n";
    stats += TCollection_ExtendedString("  BSplines: ") + m_curveStats3D.bsplineCount + "\n";
    stats += TCollection_ExtendedString("  Beziers: ") + m_curveStats3D.bezierCount + "\n";
    stats += TCollection_ExtendedString("  Trimmed: ") + m_curveStats3D.trimmedCount + "\n";
    stats += TCollection_ExtendedString("  Other: ") + m_curveStats3D.otherCount + "\n";
    stats += TCollection_ExtendedString("  Total Unique: ") + m_uniqueCurves3D.Extent() + "\n\n";
    
    stats += "2D Curves Statistics:\n";
    stats += TCollection_ExtendedString("  Lines: ") + m_curveStats2D.lineCount + "\n";
    stats += TCollection_ExtendedString("  Circles: ") + m_curveStats2D.circleCount + "\n";
    stats += TCollection_ExtendedString("  Ellipses: ") + m_curveStats2D.ellipseCount + "\n";
    stats += TCollection_ExtendedString("  BSplines: ") + m_curveStats2D.bsplineCount + "\n";
    stats += TCollection_ExtendedString("  Beziers: ") + m_curveStats2D.bezierCount + "\n";
    stats += TCollection_ExtendedString("  Trimmed: ") + m_curveStats2D.trimmedCount + "\n";
    stats += TCollection_ExtendedString("  Other: ") + m_curveStats2D.otherCount + "\n";
    stats += TCollection_ExtendedString("  Total Unique: ") + m_uniqueCurves2D.Extent();
}

// 获取拓扑类型统计信息
void TopologyExplorer::GetTopologyStatistics(TCollection_ExtendedString& stats) const
{
    stats.Clear();
    stats += "Topology Statistics:\n";
    
    const char* typeNames[] = {
        "COMPOUND", "COMPSOLID", "SOLID", "SHELL", "FACE",
        "WIRE", "EDGE", "VERTEX", "SHAPE"
    };
    
    for (int i = 0; i < TopAbs_SHAPE; i++) {
        int count = m_shapeMaps[i].Extent();
        if (count > 0) {
            stats += TCollection_ExtendedString("  ") + typeNames[i] + ": " + count + "\n";
        }
    }
}

// 清除所有数据
void TopologyExplorer::Clear()
{
    for (int i = 0; i < TopAbs_SHAPE; i++) {
        m_shapeMaps[i].Clear();
    }
    
    m_uniqueCurves3D.Clear();
    m_uniqueCurves2D.Clear();
    
    // 重置统计
    m_curveStats3D = CurveStats();
    m_curveStats2D = CurveStats();
}

// 导出所有曲线到STEP文件
bool TopologyExplorer::ExportCurvesToSTEP(const TCollection_ExtendedString& filename) const
{
    STEPControl_Writer writer;
    
    // 为每条3D曲线创建一个边并添加到STEP
    for (int i = 1; i <= m_uniqueCurves3D.Extent(); i++) {
        Handle(Geom_Curve) curve = Handle(Geom_Curve)::DownCast(m_uniqueCurves3D.FindKey(i));
        if (!curve.IsNull()) {
            // 为曲线创建一个边（使用曲线的定义域）
            BRepBuilderAPI_MakeEdge edgeMaker(curve, curve->FirstParameter(), curve->LastParameter());
            if (edgeMaker.IsDone()) {
                TopoDS_Edge edge = edgeMaker.Edge();
                writer.Transfer(edge, STEPControl_AsIs);
            }
        }
    }
    
    // 写入文件
    Standard_Integer filenameLen = filename.LengthOfCString();
    char* buffer = new char[filenameLen + 1];
    filename.ToUTF8CString(reinterpret_cast<Standard_PCharacter&>(buffer));
    IFSelect_ReturnStatus status = writer.Write(buffer);
    delete[] buffer;
    return (status == IFSelect_RetDone);
}

// 查找特定类型的曲线
std::vector<Handle(Geom_Curve)> TopologyExplorer::FindCurvesByType(Standard_CString typeName) const
{
    std::vector<Handle(Geom_Curve)> result;
    
    for (int i = 1; i <= m_uniqueCurves3D.Extent(); i++) {
        Handle(Geom_Curve) curve = Handle(Geom_Curve)::DownCast(m_uniqueCurves3D.FindKey(i));
        if (!curve.IsNull()) {
            TCollection_ExtendedString curveType = GetCurveTypeName(curve);
            if (curveType.IsEqual(typeName)) {
                result.push_back(curve);
            }
        }
    }
    
    return result;
}

// 获取曲线类型名称
TCollection_ExtendedString TopologyExplorer::GetCurveTypeName(const Handle(Geom_Curve)& curve) const
{
    if (curve.IsNull()) return "Null";
    
    if (curve->DynamicType() == STANDARD_TYPE(Geom_Line)) {
        return "Line";
    }
    else if (curve->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
        return "Circle";
    }
    else if (curve->DynamicType() == STANDARD_TYPE(Geom_Ellipse)) {
        return "Ellipse";
    }
    else if (curve->DynamicType() == STANDARD_TYPE(Geom_BSplineCurve)) {
        return "BSplineCurve";
    }
    else if (curve->DynamicType() == STANDARD_TYPE(Geom_BezierCurve)) {
        return "BezierCurve";
    }
    else if (curve->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve)) {
        return "TrimmedCurve";
    }
    
    return curve->DynamicType()->Name();
}

// 获取2D曲线类型名称
TCollection_ExtendedString TopologyExplorer::GetCurve2DTypeName(const Handle(Geom2d_Curve)& curve) const
{
    if (curve.IsNull()) return "Null";
    
    if (curve->DynamicType() == STANDARD_TYPE(Geom2d_Line)) {
        return "Line2d";
    }
    else if (curve->DynamicType() == STANDARD_TYPE(Geom2d_Circle)) {
        return "Circle2d";
    }
    else if (curve->DynamicType() == STANDARD_TYPE(Geom2d_Ellipse)) {
        return "Ellipse2d";
    }
    else if (curve->DynamicType() == STANDARD_TYPE(Geom2d_BSplineCurve)) {
        return "BSplineCurve2d";
    }
    else if (curve->DynamicType() == STANDARD_TYPE(Geom2d_BezierCurve)) {
        return "BezierCurve2d";
    }
    else if (curve->DynamicType() == STANDARD_TYPE(Geom2d_TrimmedCurve)) {
        return "TrimmedCurve2d";
    }
    
    return curve->DynamicType()->Name();
}

// 调试：打印所有信息
void TopologyExplorer::PrintInfo() const
{
    TCollection_ExtendedString topoStats, curveStats;
    GetTopologyStatistics(topoStats);
    GetCurveStatistics(curveStats);
    
    std::cout << "=== OCCT Topology Explorer Report ===\n" << std::endl;
    Standard_Integer topoLen = topoStats.LengthOfCString();
    char* topoStatsBuffer = new char[topoLen + 1];
    topoStats.ToUTF8CString(reinterpret_cast<Standard_PCharacter&>(topoStatsBuffer));
    
    Standard_Integer crvLen = curveStats.LengthOfCString();
    char* curveStatsBuffer = new char[crvLen + 1];
    curveStats.ToUTF8CString(reinterpret_cast<Standard_PCharacter&>(curveStatsBuffer));
    
    std::cout << topoStatsBuffer << std::endl;
    std::cout << curveStatsBuffer << std::endl;
    
    delete[] topoStatsBuffer;
    delete[] curveStatsBuffer;
    
    // 打印所有3D曲线详细信息
    std::cout << "\n=== Detailed 3D Curves List ===\n" << std::endl;
    for (int i = 1; i <= m_uniqueCurves3D.Extent(); i++) {
        Handle(Geom_Curve) curve = Handle(Geom_Curve)::DownCast(m_uniqueCurves3D.FindKey(i));
        if (!curve.IsNull()) {
            TCollection_ExtendedString typeName = GetCurveTypeName(curve);
            
            // 使用GeomAdaptor_Curve和GCPnts_UniformDeflection计算曲线长度
            GeomAdaptor_Curve adaptor(curve);
            double length = 0.0;
            
            // 计算曲线长度的替代方法：使用GCPnts_UniformDeflection的构造函数和迭代器
            GCPnts_UniformDeflection uniformDeflection(adaptor, 1e-6);
            if (uniformDeflection.IsDone()) {
                // 简单的长度计算：累加所有分段的距离
                if (uniformDeflection.NbPoints() > 1) {
                    for (int j = 1; j < uniformDeflection.NbPoints(); j++) {
                        gp_Pnt p1 = uniformDeflection.Value(j);
                        gp_Pnt p2 = uniformDeflection.Value(j + 1);
                        length += p1.Distance(p2);
                    }
                }
            }
            
            Standard_Integer typeNameLen = typeName.LengthOfCString();
            char* typeNameBuffer = new char[typeNameLen + 1];
            typeName.ToUTF8CString(reinterpret_cast<Standard_PCharacter&>(typeNameBuffer));
            std::cout << "Curve " << i << ": " << typeNameBuffer
                      << ", Length: " << length
                      << ", Range: [" << curve->FirstParameter()
                      << ", " << curve->LastParameter() << "]"
                      << ", Address: " << curve.get() << std::endl;
            delete[] typeNameBuffer;
        }
    }
}