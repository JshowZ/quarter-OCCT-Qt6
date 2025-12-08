#ifndef SHAPE_STATISTICS_H
#define SHAPE_STATISTICS_H

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopAbs.hxx>
#include <TopTools_ListOfShape.hxx>

class ShapeStatistics {
public:
    explicit ShapeStatistics();
    ~ShapeStatistics();
    
    // 统计形状中所有元素
    void computeStatistics(const TopoDS_Shape& inputShape);
    
    // 重置统计结果
    void reset();
    
    // 获取统计结果
    int getVertexCount() const;
    int getEdgeCount() const;
    int getWireCount() const;
    int getFaceCount() const;
    int getShellCount() const;
    int getSolidCount() const;
    int getCompSolidCount() const;
    int getCompoundCount() const;
    int getTotalShapeCount() const;
    
    // 获取特定类型的形状数量
    int getShapeCount(TopAbs_ShapeEnum shapeType) const;
    
    // 打印统计结果
    void printStatistics() const;
    
    // 检查形状是否包含特定类型的元素
    bool hasShapeType(const TopoDS_Shape& inputShape, TopAbs_ShapeEnum shapeType) const;
    
    // 获取最复杂的形状类型
    TopAbs_ShapeEnum getMostComplexShapeType() const;
    
    // 获取存储的曲线（边）
    const TopTools_ListOfShape& getExtractedEdges() const;
    
    // 获取存储的特定类型的形状
    void getShapesOfType(TopAbs_ShapeEnum shapeType, TopTools_ListOfShape& shapes) const;
    
    // 保存提取到的曲线到BREP文件
    bool saveEdgesToBREP(const std::string& filePath) const;
    
    // 保存特定类型的形状到BREP文件
    bool saveShapesToBREP(TopAbs_ShapeEnum shapeType, const std::string& filePath) const;
    
private:
    // 递归处理形状
    void processShapeRecursive(const TopoDS_Shape& shape);
    
    // 更新形状计数
    void updateCount(TopAbs_ShapeEnum shapeType);
    
    // 各个形状类型的计数
    int vertexCount_;
    int edgeCount_;
    int wireCount_;
    int faceCount_;
    int shellCount_;
    int solidCount_;
    int compSolidCount_;
    int compoundCount_;
    int totalCount_;
    
    // 最复杂的形状类型
    TopAbs_ShapeEnum mostComplexType_;
    
    // 存储所有提取的形状
    TopTools_ListOfShape extractedEdges_;
    TopTools_ListOfShape extractedVertices_;
    TopTools_ListOfShape extractedWires_;
    TopTools_ListOfShape extractedFaces_;
    TopTools_ListOfShape extractedShells_;
    TopTools_ListOfShape extractedSolids_;
    TopTools_ListOfShape extractedCompSolids_;
    TopTools_ListOfShape extractedCompounds_;
};

#endif // SHAPE_STATISTICS_H