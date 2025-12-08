#include "MeshRemover.h"

// OCCT Includes
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_CompSolid.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Wire.hxx>

MeshRemover::MeshRemover(double deflection, double angle)
    : deflection_(deflection)
    , angle_(angle)
    , meshableCount_(0)
    , nonMeshableCount_(0)
{
    // 初始化复合形状
    BRep_Builder localBuilder;
    localBuilder.MakeCompound(meshableParts_);
    localBuilder.MakeCompound(nonMeshableParts_);
}

MeshRemover::~MeshRemover()
{
}

bool MeshRemover::removeMeshableParts(const TopoDS_Shape& inputShape, TopoDS_Shape& outputShape)
{
    if (inputShape.IsNull()) {
        outputShape.Nullify();
        return false;
    }
    
    // 重置统计信息
    meshableCount_ = 0;
    nonMeshableCount_ = 0;
    
    // 清空之前的结果
    BRep_Builder localBuilder;
    localBuilder.MakeCompound(meshableParts_);
    localBuilder.MakeCompound(nonMeshableParts_);
    
    // 递归处理形状
    processShapeRecursive(inputShape, nonMeshableParts_);
    
    // 设置输出形状为不可三角剖分的部分
    outputShape = nonMeshableParts_;
    
    return true;
}

void MeshRemover::setDeflection(double deflection)
{
    deflection_ = deflection;
}

void MeshRemover::setAngle(double angle)
{
    angle_ = angle;
}

double MeshRemover::getDeflection() const
{
    return deflection_;
}

double MeshRemover::getAngle() const
{
    return angle_;
}

int MeshRemover::getMeshablePartCount() const
{
    return meshableCount_;
}

int MeshRemover::getNonMeshablePartCount() const
{
    return nonMeshableCount_;
}

const TopoDS_Compound& MeshRemover::getMeshableParts() const
{
    return meshableParts_;
}

const TopoDS_Compound& MeshRemover::getNonMeshableParts() const
{
    return nonMeshableParts_;
}

bool MeshRemover::isMeshable(const TopoDS_Shape& shape)
{
    if (shape.IsNull()) {
        return false;
    }
    
    // 跳过顶点和边（无法单独三角剖分）
    TopAbs_ShapeEnum shapeType = shape.ShapeType();
    if (shapeType == TopAbs_VERTEX || shapeType == TopAbs_EDGE) {
        return false;
    }
    
    // 尝试三角剖分
    if (!meshShape(shape)) {
        return false;
    }
    
    // 检查是否有有效的三角剖分
    return hasValidTriangulation(shape);
}

bool MeshRemover::meshShape(const TopoDS_Shape& shape)
{
    try {
        // 创建三角剖分器
        BRepMesh_IncrementalMesh meshBuilder(shape, deflection_, Standard_False, angle_, Standard_True);
        meshBuilder.Perform();
        
        return meshBuilder.IsDone();
    } catch (...) {
        return false;
    }
}

bool MeshRemover::hasValidTriangulation(const TopoDS_Shape& shape)
{
    try {
        bool hasTriangles = false;
        
        // 探索所有面
        TopExp_Explorer faceExplorer(shape, TopAbs_FACE);
        while (faceExplorer.More()) {
            const TopoDS_Face& face = TopoDS::Face(faceExplorer.Current());
            
            // 获取面的三角剖分
            TopLoc_Location loc;
            Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, loc);
            
            if (!triangulation.IsNull() && triangulation->NbTriangles() > 0) {
                hasTriangles = true;
                break;
            }
            
            faceExplorer.Next();
        }
        
        return hasTriangles;
    } catch (...) {
        return false;
    }
}

void MeshRemover::processShapeRecursive(const TopoDS_Shape& shape, TopoDS_Compound& nonMeshableParts)
{
    if (shape.IsNull()) {
        return;
    }
    
    // 检查当前形状是否可以三角剖分
    if (isMeshable(shape)) {
        // 如果可以三角剖分，添加到可三角剖分部分
        BRep_Builder localBuilder;
        localBuilder.Add(meshableParts_, shape);
        meshableCount_++;
        return;
    }
    
    // 当前形状不能三角剖分，检查是否可以分解
    TopAbs_ShapeEnum shapeType = shape.ShapeType();
    bool decomposed = false;
    
    switch (shapeType) {
    case TopAbs_COMPOUND:
    {
        // 分解复合体
        decomposed = true;
        removeMeshableFromCompound(TopoDS::Compound(shape), nonMeshableParts);
        break;
    }
    case TopAbs_COMPSOLID:
    {
        decomposed = true;
        // 分解复合实体
        TopExp_Explorer solidExplorer(shape, TopAbs_SOLID);
        while (solidExplorer.More()) {
            const TopoDS_Shape& solid = solidExplorer.Current();
            processShapeRecursive(solid, nonMeshableParts);
            solidExplorer.Next();
        }
        break;
    }
    case TopAbs_SOLID:
    {
        decomposed = true;
        // 分解实体
        TopExp_Explorer shellExplorer(shape, TopAbs_SHELL);
        while (shellExplorer.More()) {
            const TopoDS_Shape& shell = shellExplorer.Current();
            processShapeRecursive(shell, nonMeshableParts);
            shellExplorer.Next();
        }
        break;
    }
    case TopAbs_SHELL:
    {
        decomposed = true;
        // 分解壳
        TopExp_Explorer faceExplorer(shape, TopAbs_FACE);
        while (faceExplorer.More()) {
            const TopoDS_Shape& face = faceExplorer.Current();
            processShapeRecursive(face, nonMeshableParts);
            faceExplorer.Next();
        }
        break;
    }
    case TopAbs_FACE:
    {
        // 面不能三角剖分，添加到不可三角剖分部分
        BRep_Builder localBuilder;
        localBuilder.Add(nonMeshableParts, shape);
        nonMeshableCount_++;
        break;
    }
    case TopAbs_WIRE:
    case TopAbs_EDGE:
    case TopAbs_VERTEX:
    {
        // 线框、边和顶点添加到不可三角剖分部分
        BRep_Builder localBuilder;
        localBuilder.Add(nonMeshableParts, shape);
        nonMeshableCount_++;
        break;
    }
    default:
    {
        // 其他形状类型，添加到不可三角剖分部分
        BRep_Builder localBuilder;
        localBuilder.Add(nonMeshableParts, shape);
        nonMeshableCount_++;
        break;
    }
    }
}

void MeshRemover::removeMeshableFromCompound(const TopoDS_Compound& inputCompound, TopoDS_Compound& outputCompound)
{
    // 遍历复合形状中的所有子形状
    TopExp_Explorer explorer(inputCompound, TopAbs_SOLID);
    while (explorer.More()) {
        const TopoDS_Shape& subShape = explorer.Current();
        processShapeRecursive(subShape, outputCompound);
        explorer.Next();
    }
    
    // 也需要遍历其他类型的子形状
    TopExp_Explorer faceExplorer(inputCompound, TopAbs_FACE);
    while (faceExplorer.More()) {
        const TopoDS_Shape& face = faceExplorer.Current();
        processShapeRecursive(face, outputCompound);
        faceExplorer.Next();
    }
    
    TopExp_Explorer edgeExplorer(inputCompound, TopAbs_EDGE);
    while (edgeExplorer.More()) {
        const TopoDS_Shape& edge = edgeExplorer.Current();
        processShapeRecursive(edge, outputCompound);
        edgeExplorer.Next();
    }
    
    TopExp_Explorer vertexExplorer(inputCompound, TopAbs_VERTEX);
    while (vertexExplorer.More()) {
        const TopoDS_Shape& vertex = vertexExplorer.Current();
        processShapeRecursive(vertex, outputCompound);
        vertexExplorer.Next();
    }
}
