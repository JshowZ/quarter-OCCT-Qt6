#include "STLMultiLevelExporter.h"

#include <BRepMesh_IncrementalMesh.hxx>
#include <StlAPI_Writer.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopAbs.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <TopLoc_Location.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <Standard_Failure.hxx>
#include <iostream>
#include <sstream>
#include <fstream>

// 辅助函数声明：将TopAbs_ShapeEnum转换为字符串
std::string shapeTypeToString(TopAbs_ShapeEnum shapeType);

// 静态常量定义
const double STLMultiLevelExporter::TRIANGULATION_COVERAGE_THRESHOLD = 0.8;

STLMultiLevelExporter::STLMultiLevelExporter(double deflection) : m_deflection(deflection) {
}

bool STLMultiLevelExporter::exportToSTL(const TopoDS_Shape& inputShape, const std::string& filename) {
    // 分解并分析模型
    std::vector<ExportResult> results = decomposeAndAnalyze(inputShape);
    
    // 收集可导出的形状
    std::vector<TopoDS_Shape> exportableShapes;
    for (const auto& result : results) {
        if (result.exportedSuccessfully) {
            exportableShapes.push_back(result.shape);
        }
    }
    
    if (exportableShapes.empty()) {
        return false;
    }
    
    // 将可导出的形状合并为一个复合形状
    BRep_Builder builder;
    TopoDS_Compound resultCompound;
    builder.MakeCompound(resultCompound);
    
    for (const auto& shape : exportableShapes) {
        builder.Add(resultCompound, shape);
    }
    
    // 导出到STL
    StlAPI_Writer stlWriter;
    stlWriter.ASCIIMode() = false;
    return stlWriter.Write(resultCompound, filename.c_str());
}

std::vector<ExportResult> STLMultiLevelExporter::decomposeAndAnalyze(const TopoDS_Shape& inputShape) {
    m_exportResults.clear();
    decomposeShape(inputShape, "ROOT", m_exportResults);
    return m_exportResults;
}

void STLMultiLevelExporter::decomposeShape(const TopoDS_Shape& shape, const std::string& currentLevel, std::vector<ExportResult>& results) {
    ExportResult result;
    result.shape = shape;
    result.shapeLevel = currentLevel;
    result.faceCount = 0;
    result.triangulatedFaceCount = 0;
    result.failureReason = "";
    
    // 尝试导出当前形状
    bool exported = tryExportShape(shape, result);
    results.push_back(result);
    
    // 如果当前形状导出失败，尝试分解为更低级别的形状
    if (!exported) {
        TopAbs_ShapeEnum shapeType = shape.ShapeType();
        
        switch (shapeType) {
        case TopAbs_COMPOUND:
        case TopAbs_COMPSOLID:
            // 分解为SOLID级别
            for (TopExp_Explorer explorer(shape, TopAbs_SOLID); explorer.More(); explorer.Next()) {
                decomposeShape(explorer.Current(), "SOLID", results);
            }
            break;
        case TopAbs_SOLID:
            // 分解为SHELL级别
            for (TopExp_Explorer explorer(shape, TopAbs_SHELL); explorer.More(); explorer.Next()) {
                decomposeShape(explorer.Current(), "SHELL", results);
            }
            break;
        case TopAbs_SHELL:
            // 分解为FACE级别
            for (TopExp_Explorer explorer(shape, TopAbs_FACE); explorer.More(); explorer.Next()) {
                decomposeShape(explorer.Current(), "FACE", results);
            }
            break;
        case TopAbs_WIRE:
        case TopAbs_EDGE:
        case TopAbs_VERTEX:
            // 这些形状类型不能单独导出到STL
            break;
        default:
            break;
        }
    }
}

bool STLMultiLevelExporter::tryExportShape(const TopoDS_Shape& shape, ExportResult& result) {
    result.exportedSuccessfully = false;
    result.faceCount = 0;
    result.triangulatedFaceCount = 0;
    result.failureReason = "";
    
    // 跳过空形状
    if (shape.IsNull()) {
        result.failureReason = "Empty shape";
        return false;
    }
    
    // 跳过不能导出到STL的形状类型
    TopAbs_ShapeEnum shapeType = shape.ShapeType();
    if (shapeType == TopAbs_VERTEX || shapeType == TopAbs_EDGE || 
        shapeType == TopAbs_WIRE) {
        result.failureReason = "Shape type cannot be exported to STL";
        return false;
    }
    
    try {
        // 尝试对形状进行网格化
        BRepMesh_IncrementalMesh mesher(shape, m_deflection, false, 0.5, true);
        mesher.Perform();
        
        if (!mesher.IsDone()) {
            result.failureReason = "Mesh generation failed";
            return false;
        }
        
        // 计算三角化覆盖率
        double coverage = calculateTriangulationCoverage(shape);
        
        // 检查覆盖率是否达到阈值
        if (coverage >= TRIANGULATION_COVERAGE_THRESHOLD) {
            result.exportedSuccessfully = true;
            return true;
        } else {
            result.failureReason = "Triangulation coverage too low";
            return false;
        }
        
    } catch (Standard_Failure& e) {
        result.failureReason = "Exception during meshing: " + std::string(e.GetMessageString());
        return false;
    } catch (...) {
        result.failureReason = "Unknown exception during meshing";
        return false;
    }
}

bool STLMultiLevelExporter::isTriangulable(const TopoDS_Shape& shape) {
    try {
        BRepMesh_IncrementalMesh mesher(shape, m_deflection, false, 0.5, true);
        mesher.Perform();
        
        if (!mesher.IsDone()) {
            return false;
        }
        
        double coverage = calculateTriangulationCoverage(shape);
        return coverage >= TRIANGULATION_COVERAGE_THRESHOLD;
        
    } catch (...) {
        return false;
    }
}

double STLMultiLevelExporter::calculateTriangulationCoverage(const TopoDS_Shape& shape) {
    int faceCount = 0;
    int triangulatedFaceCount = 0;
    
    // 遍历所有面，计算三角化覆盖率
    for (TopExp_Explorer faceExplorer(shape, TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {
        faceCount++;
        TopoDS_Face face = TopoDS::Face(faceExplorer.Current());
        TopLoc_Location location;
        
        // 获取面的三角化结果
        Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);
        
        if (!triangulation.IsNull() && triangulation->NbTriangles() > 0) {
            triangulatedFaceCount++;
        }
    }
    
    // 更新结果对象中的面计数
    ExportResult* result = &m_exportResults.back();
    result->faceCount = faceCount;
    result->triangulatedFaceCount = triangulatedFaceCount;
    
    if (faceCount == 0) {
        return 0.0;
    }
    
    return static_cast<double>(triangulatedFaceCount) / faceCount;
}

void STLMultiLevelExporter::setDeflection(double deflection) {
    m_deflection = deflection;
}

double STLMultiLevelExporter::getDeflection() const {
    return m_deflection;
}

const std::vector<ExportResult>& STLMultiLevelExporter::getExportResults() const {
    return m_exportResults;
}

std::string STLMultiLevelExporter::generateReport() const {
    std::stringstream report;
    
    report << "=== STL Multi-Level Export Report ===\n";
    report << "Deflection: " << m_deflection << "\n";
    report << "Total shapes processed: " << m_exportResults.size() << "\n";
    report << "Successfully exported: " << getSuccessCount() << "\n";
    report << "Failed to export: " << getFailureCount() << "\n\n";
    
    int successCounter = 0;
    int failureCounter = 0;
    
    for (size_t i = 0; i < m_exportResults.size(); i++) {
        const ExportResult& result = m_exportResults[i];
        
        if (result.exportedSuccessfully) {
            report << "✓ SUCCESS #" << ++successCounter << " (" << result.shapeLevel << ")\n";
        } else {
            report << "✗ FAILURE #" << ++failureCounter << " (" << result.shapeLevel << ")\n";
        }
        
        report << "  Shape type: " << shapeTypeToString(result.shape.ShapeType()) << "\n";
        report << "  Faces: " << result.triangulatedFaceCount << "/" << result.faceCount << " (" 
               << static_cast<int>(static_cast<double>(result.triangulatedFaceCount) / result.faceCount * 100) << "%)\n";
        
        if (!result.exportedSuccessfully) {
            report << "  Reason: " << result.failureReason << "\n";
        }
        
        report << "\n";
    }
    
    return report.str();
}

int STLMultiLevelExporter::getSuccessCount() const {
    int count = 0;
    for (const auto& result : m_exportResults) {
        if (result.exportedSuccessfully) {
            count++;
        }
    }
    return count;
}

int STLMultiLevelExporter::getFailureCount() const {
    int count = 0;
    for (const auto& result : m_exportResults) {
        if (!result.exportedSuccessfully) {
            count++;
        }
    }
    return count;
}

int STLMultiLevelExporter::getTotalShapeCount() const {
    return m_exportResults.size();
}

// 辅助函数：将TopAbs_ShapeEnum转换为字符串
std::string shapeTypeToString(TopAbs_ShapeEnum shapeType) {
    switch (shapeType) {
    case TopAbs_VERTEX:
        return "VERTEX";
    case TopAbs_EDGE:
        return "EDGE";
    case TopAbs_WIRE:
        return "WIRE";
    case TopAbs_FACE:
        return "FACE";
    case TopAbs_SHELL:
        return "SHELL";
    case TopAbs_SOLID:
        return "SOLID";
    case TopAbs_COMPSOLID:
        return "COMPSOLID";
    case TopAbs_COMPOUND:
        return "COMPOUND";
    default:
        return "UNKNOWN";
    }
}