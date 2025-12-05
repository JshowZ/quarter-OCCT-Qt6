#include "ShapeStatistics.h"

// OCCT Includes
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_CompSolid.hxx>
#include <TopoDS_Compound.hxx>

// Standard Includes
#include <iostream>

ShapeStatistics::ShapeStatistics()
    : vertexCount_(0)
    , edgeCount_(0)
    , wireCount_(0)
    , faceCount_(0)
    , shellCount_(0)
    , solidCount_(0)
    , compSolidCount_(0)
    , compoundCount_(0)
    , totalCount_(0)
    , mostComplexType_(TopAbs_VERTEX)
{
}

ShapeStatistics::~ShapeStatistics()
{
}

void ShapeStatistics::computeStatistics(const TopoDS_Shape& inputShape)
{
    reset();
    processShapeRecursive(inputShape);
}

void ShapeStatistics::reset()
{
    vertexCount_ = 0;
    edgeCount_ = 0;
    wireCount_ = 0;
    faceCount_ = 0;
    shellCount_ = 0;
    solidCount_ = 0;
    compSolidCount_ = 0;
    compoundCount_ = 0;
    totalCount_ = 0;
    mostComplexType_ = TopAbs_VERTEX;
}

int ShapeStatistics::getVertexCount() const
{
    return vertexCount_;
}

int ShapeStatistics::getEdgeCount() const
{
    return edgeCount_;
}

int ShapeStatistics::getWireCount() const
{
    return wireCount_;
}

int ShapeStatistics::getFaceCount() const
{
    return faceCount_;
}

int ShapeStatistics::getShellCount() const
{
    return shellCount_;
}

int ShapeStatistics::getSolidCount() const
{
    return solidCount_;
}

int ShapeStatistics::getCompSolidCount() const
{
    return compSolidCount_;
}

int ShapeStatistics::getCompoundCount() const
{
    return compoundCount_;
}

int ShapeStatistics::getTotalShapeCount() const
{
    return totalCount_;
}

int ShapeStatistics::getShapeCount(TopAbs_ShapeEnum shapeType) const
{
    switch (shapeType) {
    case TopAbs_VERTEX:
        return vertexCount_;
    case TopAbs_EDGE:
        return edgeCount_;
    case TopAbs_WIRE:
        return wireCount_;
    case TopAbs_FACE:
        return faceCount_;
    case TopAbs_SHELL:
        return shellCount_;
    case TopAbs_SOLID:
        return solidCount_;
    case TopAbs_COMPSOLID:
        return compSolidCount_;
    case TopAbs_COMPOUND:
        return compoundCount_;
    default:
        return 0;
    }
}

void ShapeStatistics::printStatistics() const
{
    std::cout << "\n=== 形状统计结果 ===" << std::endl;
    std::cout << "顶点 (Vertex): " << vertexCount_ << std::endl;
    std::cout << "边 (Edge): " << edgeCount_ << std::endl;
    std::cout << "线框 (Wire): " << wireCount_ << std::endl;
    std::cout << "面 (Face): " << faceCount_ << std::endl;
    std::cout << "壳 (Shell): " << shellCount_ << std::endl;
    std::cout << "实体 (Solid): " << solidCount_ << std::endl;
    std::cout << "复合实体 (CompSolid): " << compSolidCount_ << std::endl;
    std::cout << "复合体 (Compound): " << compoundCount_ << std::endl;
    std::cout << "总形状数: " << totalCount_ << std::endl;
    std::cout << "====================\n" << std::endl;
}

bool ShapeStatistics::hasShapeType(const TopoDS_Shape& inputShape, TopAbs_ShapeEnum shapeType) const
{
    TopExp_Explorer explorer(inputShape, shapeType);
    return explorer.More();
}

TopAbs_ShapeEnum ShapeStatistics::getMostComplexShapeType() const
{
    return mostComplexType_;
}

void ShapeStatistics::processShapeRecursive(const TopoDS_Shape& shape)
{
    if (shape.IsNull()) {
        return;
    }
    
    // 更新当前形状的计数
    TopAbs_ShapeEnum shapeType = shape.ShapeType();
    updateCount(shapeType);
    
    // 更新最复杂形状类型
    if (shapeType > mostComplexType_) {
        mostComplexType_ = shapeType;
    }
    
    // 根据形状类型，探索相应的子形状类型
    TopAbs_ShapeEnum subShapeType;
    
    switch (shapeType) {
    case TopAbs_COMPOUND:
        subShapeType = TopAbs_SOLID;
        break;
    case TopAbs_COMPSOLID:
        subShapeType = TopAbs_SOLID;
        break;
    case TopAbs_SOLID:
        subShapeType = TopAbs_SHELL;
        break;
    case TopAbs_SHELL:
        subShapeType = TopAbs_FACE;
        break;
    case TopAbs_FACE:
        subShapeType = TopAbs_WIRE;
        break;
    case TopAbs_WIRE:
        subShapeType = TopAbs_EDGE;
        break;
    case TopAbs_EDGE:
        subShapeType = TopAbs_VERTEX;
        break;
    default:
        // 没有子形状可以探索
        return;
    }
    
    // 遍历所有子形状
    TopExp_Explorer explorer(shape, subShapeType);
    while (explorer.More()) {
        const TopoDS_Shape& subShape = explorer.Current();
        processShapeRecursive(subShape);
        explorer.Next();
    }
}

void ShapeStatistics::updateCount(TopAbs_ShapeEnum shapeType)
{
    switch (shapeType) {
    case TopAbs_VERTEX:
        vertexCount_++;
        break;
    case TopAbs_EDGE:
        edgeCount_++;
        break;
    case TopAbs_WIRE:
        wireCount_++;
        break;
    case TopAbs_FACE:
        faceCount_++;
        break;
    case TopAbs_SHELL:
        shellCount_++;
        break;
    case TopAbs_SOLID:
        solidCount_++;
        break;
    case TopAbs_COMPSOLID:
        compSolidCount_++;
        break;
    case TopAbs_COMPOUND:
        compoundCount_++;
        break;
    default:
        break;
    }
    
    totalCount_++;
}