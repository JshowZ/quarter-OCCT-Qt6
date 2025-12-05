#include "CurveExtractor.h"

// OCCT Includes
#include <TopExp_Explorer.hxx>
#include <TopAbs.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Curve.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Parabola.hxx>
#include <Geom_Hyperbola.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BSplineCurve.hxx>

// Standard Includes
#include <iostream>
#include <string>

CurveExtractor::CurveExtractor()
    : edgeCount_(0)
{
}

CurveExtractor::~CurveExtractor()
{
}

void CurveExtractor::extractCurves(const TopoDS_Shape& inputShape, TopTools_ListOfShape& extractedEdges)
{
    edgeCount_ = 0;
    processShapeRecursive(inputShape, extractedEdges);
}

void CurveExtractor::extractGeometryCurves(const TopoDS_Shape& inputShape, Handle(TopTools_HSequenceOfShape)& extractedEdges,
                                         Handle(TopTools_HSequenceOfShape)& extractedCurves)
{
    if (extractedEdges.IsNull()) {
        extractedEdges = new TopTools_HSequenceOfShape();
    }
    if (extractedCurves.IsNull()) {
        extractedCurves = new TopTools_HSequenceOfShape();
    }
    
    edgeCount_ = 0;
    
    // 首先提取所有边
    TopTools_ListOfShape edgesList;
    processShapeRecursive(inputShape, edgesList);
    
    // 遍历所有边，提取几何曲线
    for (TopTools_ListIteratorOfListOfShape it(edgesList); it.More(); it.Next()) {
        const TopoDS_Shape& shape = it.Value();
        if (shape.ShapeType() == TopAbs_EDGE) {
            TopoDS_Edge edge = TopoDS::Edge(shape);
            extractedEdges->Append(edge);
            
            // 提取几何曲线
            Standard_Real firstParam, lastParam;
            Handle(Geom_Curve) curve = extractCurveFromEdge(edge, firstParam, lastParam);
            if (!curve.IsNull()) {
                extractedCurves->Append(shape); // 这里可以返回曲线的表示，或者直接使用曲线对象
            }
        }
    }
}

void CurveExtractor::extractCurveGeometryInfo(const TopoDS_Shape& inputShape, 
                                             Handle(TopTools_HSequenceOfShape)& extractedEdges, 
                                             std::vector<CurveGeometryInfo>& curveInfos)
{
    if (extractedEdges.IsNull()) {
        extractedEdges = new TopTools_HSequenceOfShape();
    }
    
    curveInfos.clear();
    edgeCount_ = 0;
    
    // 首先提取所有边
    TopTools_ListOfShape edgesList;
    processShapeRecursive(inputShape, edgesList);
    
    // 遍历所有边，提取几何信息
    for (TopTools_ListIteratorOfListOfShape it(edgesList); it.More(); it.Next()) {
        const TopoDS_Shape& shape = it.Value();
        if (shape.ShapeType() == TopAbs_EDGE) {
            TopoDS_Edge edge = TopoDS::Edge(shape);
            extractedEdges->Append(edge);
            
            // 获取曲线几何信息
            CurveGeometryInfo info = getCurveGeometryInfo(edge);
            curveInfos.push_back(info);
        }
    }
}

CurveGeometryInfo CurveExtractor::getCurveGeometryInfo(const TopoDS_Edge& edge)
{
    CurveGeometryInfo info;
    info.edge = edge;
    
    if (edge.IsNull()) {
        return info;
    }
    
    try {
        // 提取曲线和参数范围
        Standard_Real firstParam, lastParam;
        Handle(Geom_Curve) curve = extractCurveFromEdge(edge, firstParam, lastParam);
        info.curve = curve;
        info.firstParam = firstParam;
        info.lastParam = lastParam;
        
        if (curve.IsNull()) {
            return info;
        }
        
        // 获取曲线类型
        info.curveType = getCurveType(curve);
        
        // 获取起始点和结束点
        info.startPoint = curve->Value(firstParam);
        info.endPoint = curve->Value(lastParam);
        
        // 计算曲线长度
        info.length = computeCurveLength(curve, firstParam, lastParam);
        
        // 检查是否闭合
        info.isClosed = BRep_Tool::IsClosed(edge);
        
        // 检查是否周期
        info.isPeriodic = curve->IsPeriodic();
        
        // 获取曲线次数和极点数量（对于NURBS曲线）
        info.degree = 0;
        info.nbPoles = 0;
        
        // 检查曲线类型，获取更多信息
        Handle(Geom_BSplineCurve) bspline = Handle(Geom_BSplineCurve)::DownCast(curve);
        if (!bspline.IsNull()) {
            info.degree = bspline->Degree();
            info.nbPoles = bspline->NbPoles();
        }
        
        Handle(Geom_BezierCurve) bezier = Handle(Geom_BezierCurve)::DownCast(curve);
        if (!bezier.IsNull()) {
            info.degree = bezier->Degree();
            info.nbPoles = bezier->NbPoles();
        }
        
    } catch (...) {
        // 处理异常
    }
    
    return info;
}

bool CurveExtractor::hasCurves(const TopoDS_Shape& inputShape)
{
    TopExp_Explorer edgeExplorer(inputShape, TopAbs_EDGE);
    return edgeExplorer.More();
}

int CurveExtractor::getEdgeCount() const
{
    return edgeCount_;
}

std::string CurveExtractor::getCurveTypeName(TopAbs_ShapeEnum curveType)
{
    switch (curveType) {
    case TopAbs_VERTEX:
        return "Vertex";
    case TopAbs_EDGE:
        return "Edge";
    case TopAbs_WIRE:
        return "Wire";
    case TopAbs_FACE:
        return "Face";
    case TopAbs_SHELL:
        return "Shell";
    case TopAbs_SOLID:
        return "Solid";
    case TopAbs_COMPSOLID:
        return "CompSolid";
    case TopAbs_COMPOUND:
        return "Compound";
    default:
        return "Unknown";
    }
}

void CurveExtractor::printCurveInfo(const CurveGeometryInfo& info)
{
    std::cout << "\n=== 曲线几何信息 ===" << std::endl;
    std::cout << "曲线类型: " << getCurveTypeName(info.curveType) << std::endl;
    
    if (!info.curve.IsNull()) {
        std::cout << "几何曲线类型: " << info.curve->DynamicType()->Name() << std::endl;
    }
    
    std::cout << "参数范围: [" << info.firstParam << ", " << info.lastParam << "]" << std::endl;
    
    std::cout << "起始点: (" << info.startPoint.X() << ", " 
              << info.startPoint.Y() << ", " 
              << info.startPoint.Z() << ")" << std::endl;
    
    std::cout << "结束点: (" << info.endPoint.X() << ", " 
              << info.endPoint.Y() << ", " 
              << info.endPoint.Z() << ")" << std::endl;
    
    std::cout << "曲线长度: " << info.length << std::endl;
    std::cout << "是否闭合: " << (info.isClosed ? "是" : "否") << std::endl;
    std::cout << "是否周期: " << (info.isPeriodic ? "是" : "否") << std::endl;
    std::cout << "曲线次数: " << info.degree << std::endl;
    std::cout << "极点数量: " << info.nbPoles << std::endl;
    std::cout << "====================\n" << std::endl;
}

void CurveExtractor::processShapeRecursive(const TopoDS_Shape& shape, TopTools_ListOfShape& extractedEdges)
{
    // 检查当前形状是否为边
    if (shape.ShapeType() == TopAbs_EDGE) {
        extractedEdges.Append(shape);
        edgeCount_++;
        return;
    }
    
    // 如果不是边，检查是否可以分解为子形状
    TopExp_Explorer explorer(shape, TopAbs_SHAPE);
    while (explorer.More()) {
        const TopoDS_Shape& subShape = explorer.Current();
        processShapeRecursive(subShape, extractedEdges);
        explorer.Next();
    }
}

Handle(Geom_Curve) CurveExtractor::extractCurveFromEdge(const TopoDS_Edge& edge, Standard_Real& firstParam, Standard_Real& lastParam)
{
    if (edge.IsNull()) {
        return nullptr;
    }
    
    try {
        return BRep_Tool::Curve(edge, firstParam, lastParam);
    } catch (...) {
        return nullptr;
    }
}

Standard_Real CurveExtractor::computeCurveLength(const Handle(Geom_Curve)& curve, Standard_Real firstParam, Standard_Real lastParam)
{
    if (curve.IsNull()) {
        return 0.0;
    }
    
    try {
        // 使用GeomAdaptor_Curve计算长度
        GeomAdaptor_Curve adaptorCurve(curve);
        return GCPnts_AbscissaPoint::Length(adaptorCurve, firstParam, lastParam);
    } catch (...) {
        return 0.0;
    }
}

TopAbs_ShapeEnum CurveExtractor::getCurveType(const Handle(Geom_Curve)& curve) const
{
    if (curve.IsNull()) {
        return TopAbs_SHAPE;
    }
    
    // 根据曲线类型返回对应的TopAbs_ShapeEnum
    if (Handle(Geom_Line)::DownCast(curve).IsNull() && 
        Handle(Geom_Circle)::DownCast(curve).IsNull() &&
        Handle(Geom_Ellipse)::DownCast(curve).IsNull() &&
        Handle(Geom_Parabola)::DownCast(curve).IsNull() &&
        Handle(Geom_Hyperbola)::DownCast(curve).IsNull() &&
        Handle(Geom_BezierCurve)::DownCast(curve).IsNull() &&
        Handle(Geom_BSplineCurve)::DownCast(curve).IsNull()) {
        return TopAbs_SHAPE;
    }
    
    return TopAbs_EDGE;
}