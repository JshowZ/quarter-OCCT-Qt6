#ifndef CURVE_EXTRACTOR_H
#define CURVE_EXTRACTOR_H

#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <Geom_Curve.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <TColgp_HArray1OfPnt.hxx>

// 曲线几何信息结构体
typedef struct {
    Handle(Geom_Curve) curve;         // 几何曲线
    TopAbs_ShapeEnum curveType;       // 曲线类型
    Standard_Real firstParam;         // 起始参数
    Standard_Real lastParam;          // 结束参数
    gp_Pnt startPoint;                // 起始点
    gp_Pnt endPoint;                  // 结束点
    Standard_Real length;             // 曲线长度
    bool isClosed;                    // 是否闭合
    bool isPeriodic;                  // 是否周期
    int degree;                       // 曲线次数（对于NURBS曲线）
    int nbPoles;                      // 极点数量（对于NURBS曲线）
    TopoDS_Edge edge;                 // 对应的边
} CurveGeometryInfo;

class CurveExtractor {
public:
    explicit CurveExtractor();
    ~CurveExtractor();
    
    // 提取形状中的所有曲线
    void extractCurves(const TopoDS_Shape& inputShape, TopTools_ListOfShape& extractedEdges);
    
    // 提取形状中的所有曲线，并返回几何曲线
    void extractGeometryCurves(const TopoDS_Shape& inputShape, Handle(TopTools_HSequenceOfShape)& extractedEdges, 
                              Handle(TopTools_HSequenceOfShape)& extractedCurves);
    
    // 提取形状中的所有曲线的几何信息
    void extractCurveGeometryInfo(const TopoDS_Shape& inputShape, 
                                 Handle(TopTools_HSequenceOfShape)& extractedEdges, 
                                 std::vector<CurveGeometryInfo>& curveInfos);
    
    // 获取单条边的几何信息
    CurveGeometryInfo getCurveGeometryInfo(const TopoDS_Edge& edge);
    
    // 检查形状是否包含曲线
    bool hasCurves(const TopoDS_Shape& inputShape);
    
    // 获取提取的边数量
    int getEdgeCount() const;
    
    // 获取曲线类型名称
    static std::string getCurveTypeName(TopAbs_ShapeEnum curveType);
    
    // 打印曲线几何信息
    static void printCurveInfo(const CurveGeometryInfo& info);
    
private:
    // 递归处理形状
    void processShapeRecursive(const TopoDS_Shape& shape, TopTools_ListOfShape& extractedEdges);
    
    // 从边中提取几何曲线
    Handle(Geom_Curve) extractCurveFromEdge(const TopoDS_Edge& edge, Standard_Real& firstParam, Standard_Real& lastParam);
    
    // 计算曲线长度
    Standard_Real computeCurveLength(const Handle(Geom_Curve)& curve, Standard_Real firstParam, Standard_Real lastParam);
    
    // 获取曲线类型
    TopAbs_ShapeEnum getCurveType(const Handle(Geom_Curve)& curve) const;
    
    int edgeCount_;
};

#endif // CURVE_EXTRACTOR_H