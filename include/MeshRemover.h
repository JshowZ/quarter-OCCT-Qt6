#ifndef MESH_REMOVER_H
#define MESH_REMOVER_H

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Builder.hxx>

class MeshRemover {
public:
    explicit MeshRemover(double deflection = 0.01, double angle = 0.5);
    ~MeshRemover();
    
    // 从模型中删除可三角剖分的部分，返回剩余部分
    bool removeMeshableParts(const TopoDS_Shape& inputShape, TopoDS_Shape& outputShape);
    
    // 设置三角剖分参数
    void setDeflection(double deflection);
    void setAngle(double angle);
    
    // 获取三角剖分参数
    double getDeflection() const;
    double getAngle() const;
    
    // 获取统计信息
    int getMeshablePartCount() const;
    int getNonMeshablePartCount() const;
    
    // 获取可三角剖分的部分
    const TopoDS_Compound& getMeshableParts() const;
    
    // 获取不可三角剖分的部分
    const TopoDS_Compound& getNonMeshableParts() const;
    
private:
    // 检查形状是否可以三角剖分
    bool isMeshable(const TopoDS_Shape& shape);
    
    // 三角剖分形状
    bool meshShape(const TopoDS_Shape& shape);
    
    // 检查形状是否有有效的三角剖分
    bool hasValidTriangulation(const TopoDS_Shape& shape);
    
    // 递归处理形状
    void processShapeRecursive(const TopoDS_Shape& shape, TopoDS_Compound& nonMeshableParts);
    
    // 从复合形状中移除可三角剖分的部分
    void removeMeshableFromCompound(const TopoDS_Compound& inputCompound, TopoDS_Compound& outputCompound);
    
    // 三角剖分参数
    double deflection_;
    double angle_;
    
    // 统计信息
    int meshableCount_;
    int nonMeshableCount_;
    
    // 存储结果
    TopoDS_Compound meshableParts_;
    TopoDS_Compound nonMeshableParts_;
};

#endif // MESH_REMOVER_H