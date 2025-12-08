#include "CompoundCurveExtractor.h"

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <TopAbs.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <iostream>

CompoundCurveExtractor::CompoundCurveExtractor() {
    reset();
}

CompoundCurveExtractor::~CompoundCurveExtractor() {
}

bool CompoundCurveExtractor::extractCurvesFromCompound(const TopoDS_Shape& inputShape) {
    reset();
    
    if (inputShape.IsNull()) {
        return false;
    }
    
    // 遍历输入形状，提取所有复合形状中的曲线
    traverseShape(inputShape);
    
    return !m_extractedCurves.IsEmpty();
}

bool CompoundCurveExtractor::saveCurvesToBREP(const std::string& filePath) const {
    if (m_extractedCurves.IsEmpty()) {
        return false;
    }
    
    try {
        // 创建一个复合形状来保存所有提取的曲线
        BRep_Builder builder;
        TopoDS_Compound curveCompound;
        builder.MakeCompound(curveCompound);
        
        // 将所有提取的曲线添加到复合形状中
        for (TopTools_ListIteratorOfListOfShape it(m_extractedCurves); it.More(); it.Next()) {
            const TopoDS_Shape& curveShape = it.Value();
            builder.Add(curveCompound, curveShape);
        }
        
        // 保存复合形状到BREP文件
        if (BRepTools::Write(curveCompound, filePath.c_str())) {
            return true;
        }
        
    } catch (const Standard_Failure& e) {
        std::cerr << "Error saving curves to BREP: " << e.GetMessageString() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error saving curves to BREP" << std::endl;
    }
    
    return false;
}

const TopTools_ListOfShape& CompoundCurveExtractor::getExtractedCurves() const {
    return m_extractedCurves;
}

int CompoundCurveExtractor::getCurveCount() const {
    return m_extractedCurves.Extent();
}

void CompoundCurveExtractor::reset() {
    m_extractedCurves.Clear();
}

bool CompoundCurveExtractor::loadAndExtractCurves(const std::string& stepFilePath) {
    reset();
    
    try {
        // 创建STEP阅读器
        STEPControl_Reader reader;
        IFSelect_ReturnStatus status = reader.ReadFile(stepFilePath.c_str());
        
        if (status != IFSelect_RetDone) {
            std::cerr << "Failed to read STEP file: " << stepFilePath << std::endl;
            return false;
        }
        
        // 传输形状
        reader.TransferRoots();
        
        // 获取第一个形状
        TopoDS_Shape shape = reader.OneShape();
        
        if (shape.IsNull()) {
            std::cerr << "No shape found in STEP file" << std::endl;
            return false;
        }
        
        // 提取曲线
        return extractCurvesFromCompound(shape);
        
    } catch (const Standard_Failure& e) {
        std::cerr << "Error loading STEP file: " << e.GetMessageString() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error loading STEP file" << std::endl;
    }
    
    return false;
}

void CompoundCurveExtractor::traverseShape(const TopoDS_Shape& shape) {
    if (shape.IsNull()) {
        return;
    }
    
    // 如果当前形状是复合形状，从其中提取曲线
    if (isCompound(shape)) {
        extractCurvesFromCompoundRecursive(shape);
    } else {
        // 否则，检查形状类型
        TopAbs_ShapeEnum shapeType = shape.ShapeType();
        
        if (shapeType == TopAbs_EDGE) {
            // 如果是边，直接添加到提取列表
            m_extractedCurves.Append(shape);
        } else if (shapeType == TopAbs_WIRE) {
            // 如果是线框，遍历其中的边
            for (TopExp_Explorer edgeExplorer(shape, TopAbs_EDGE); edgeExplorer.More(); edgeExplorer.Next()) {
                m_extractedCurves.Append(edgeExplorer.Current());
            }
        } else if (shapeType == TopAbs_FACE) {
            // 如果是面，遍历其中的边
            for (TopExp_Explorer edgeExplorer(shape, TopAbs_EDGE); edgeExplorer.More(); edgeExplorer.Next()) {
                m_extractedCurves.Append(edgeExplorer.Current());
            }
        } else if (shapeType == TopAbs_SHELL || shapeType == TopAbs_SOLID || 
                   shapeType == TopAbs_COMPSOLID) {
            // 对于更复杂的形状，递归遍历其中的子形状
            for (TopExp_Explorer subShapeExplorer(shape, TopAbs_ShapeEnum()); subShapeExplorer.More(); subShapeExplorer.Next()) {
                traverseShape(subShapeExplorer.Current());
            }
        }
    }
}

void CompoundCurveExtractor::extractCurvesFromCompoundRecursive(const TopoDS_Shape& compoundShape) {
    // 遍历复合形状中的所有子形状
    for (TopExp_Explorer explorer(compoundShape, TopAbs_ShapeEnum()); explorer.More(); explorer.Next()) {
        const TopoDS_Shape& subShape = explorer.Current();
        
        // 递归处理子形状
        traverseShape(subShape);
    }
}

bool CompoundCurveExtractor::isCompound(const TopoDS_Shape& shape) const {
    if (shape.IsNull()) {
        return false;
    }
    
    TopAbs_ShapeEnum shapeType = shape.ShapeType();
    return (shapeType == TopAbs_COMPOUND);
}
