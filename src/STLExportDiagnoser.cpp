#include "STLExportDiagnoser.h"

#include <STEPControl_Reader.hxx>
#include <StlAPI_Writer.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <TopLoc_Location.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <Standard_Failure.hxx>
#include <TColStd_HSequenceOfTransient.hxx>
#include <Interface_InterfaceModel.hxx>
#include <Transfer_TransientProcess.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <iostream>

STLExportDiagnoser::STLExportDiagnoser(double aDeflection) : myDeflection(aDeflection)
{
}

std::vector<MeshDiagnosis> STLExportDiagnoser::diagnoseShape(const TopoDS_Shape& aShape) 
{
    return diagnoseAndMeshShapes(aShape);
}

bool STLExportDiagnoser::diagnoseStepFile(const std::string& filename, std::vector<MeshDiagnosis>& outDiagnoses)
{
    std::vector<TopoDS_Shape> separateShapes;
    if (!readStepAsSeparateShapes(filename, separateShapes)) {
        return false;
    }
    
    for (const auto& shape : separateShapes) {
        std::vector<MeshDiagnosis> diagnoses = diagnoseAndMeshShapes(shape);
        outDiagnoses.insert(outDiagnoses.end(), diagnoses.begin(), diagnoses.end());
    }
    
    return true;
}

std::vector<TopoDS_Shape> STLExportDiagnoser::getExportableShapes(const std::vector<MeshDiagnosis>& diagnoses) const 
{
    std::vector<TopoDS_Shape> exportable;
    for (const auto& diag : diagnoses) {
        if (diag.isMeshable) {
            exportable.push_back(diag.shape);
        }
    }
    return exportable;
}

std::vector<TopoDS_Shape> STLExportDiagnoser::getNonExportableShapes(const std::vector<MeshDiagnosis>& diagnoses) const 
{
    std::vector<TopoDS_Shape> nonExportable;
    for (const auto& diag : diagnoses) {
        if (!diag.isMeshable) {
            nonExportable.push_back(diag.shape);
        }
    }
    return nonExportable;
}

bool STLExportDiagnoser::exportToSTL(const std::vector<TopoDS_Shape>& shapes, const std::string& filename) const 
{
    if (shapes.empty()) {
        return false;
    }
    
    // 将可导出的形状合并为一个复合形状
    BRep_Builder builder;
    TopoDS_Compound resultCompound;
    builder.MakeCompound(resultCompound);
    
    for (const auto& shape : shapes) {
        builder.Add(resultCompound, shape);
    }
    
    // 导出到STL
    StlAPI_Writer stlWriter;
    return stlWriter.Write(resultCompound, filename.c_str());
}

std::string STLExportDiagnoser::generateReport(const std::vector<MeshDiagnosis>& diagnoses) const
{
    std::string report;
    
    report += "=== STL导出诊断报告 ===\n";
    report += "网格化精度: " + std::to_string(myDeflection) + "\n";
    report += "总处理实体数: " + std::to_string(diagnoses.size()) + "\n\n";
    
    int meshableCount = 0;
    for (const auto& diag : diagnoses) {
        if (diag.isMeshable) meshableCount++;
    }
    
    report += "可导出实体数: " + std::to_string(meshableCount) + "\n";
    report += "不可导出实体数: " + std::to_string(diagnoses.size() - meshableCount) + "\n\n";
    
    if (diagnoses.size() - meshableCount > 0) {
        report += "不可导出实体详情:\n";
        for (size_t i = 0; i < diagnoses.size(); i++) {
            if (!diagnoses[i].isMeshable) {
                report += "  实体 " + std::to_string(i+1) + ": " + diagnoses[i].failureReason + "\n";
            }
        }
        report += "\n";
    }
    
    report += "详细诊断结果:\n";
    for (size_t i = 0; i < diagnoses.size(); i++) {
        const auto& diag = diagnoses[i];
        report += "  实体 " + std::to_string(i+1) + ":\n";
        report += "    状态: " + (diag.isMeshable ? std::string("✓ 可导出") : std::string("✗ 不可导出")) + "\n";
        if (!diag.isMeshable) {
            report += "    原因: " + diag.failureReason + "\n";
        }
        report += "    面总数: " + std::to_string(diag.faceCount) + "\n";
        report += "    成功三角化面数: " + std::to_string(diag.triangulatedFaceCount) + "\n";
        report += "    三角化覆盖率: " + std::to_string(static_cast<float>(diag.triangulatedFaceCount) / diag.faceCount * 100) + "%\n\n";
    }
    
    return report;
}

void STLExportDiagnoser::setDeflection(double aDeflection)
{
    myDeflection = aDeflection;
}

double STLExportDiagnoser::getDeflection() const
{
    return myDeflection;
}

std::vector<MeshDiagnosis> STLExportDiagnoser::diagnoseAndMeshShapes(const TopoDS_Shape& aShape) 
{
    std::vector<MeshDiagnosis> results;
    
    // 将复合模型分解为独立的实体（如SOLID, SHELL, FACE）
    // 首先在实体(SOLID)级别进行处理
    for (TopExp_Explorer solidExplorer(aShape, TopAbs_SOLID);
         solidExplorer.More();
         solidExplorer.Next()) {
        
        TopoDS_Shape currentShape = solidExplorer.Current();
        results.push_back(diagnoseSingleEntity(currentShape));
    }
    
    // 如果模型中没有SOLID级别实体，则尝试在SHELL级别处理
    if (results.empty()) {
        for (TopExp_Explorer shellExplorer(aShape, TopAbs_SHELL);
             shellExplorer.More();
             shellExplorer.Next()) {
            
            TopoDS_Shape currentShape = shellExplorer.Current();
            results.push_back(diagnoseSingleEntity(currentShape));
        }
    }
    
    // 如果模型中没有SHELL级别实体，则尝试在FACE级别处理
    if (results.empty()) {
        for (TopExp_Explorer faceExplorer(aShape, TopAbs_FACE);
             faceExplorer.More();
             faceExplorer.Next()) {
            
            TopoDS_Shape currentShape = faceExplorer.Current();
            results.push_back(diagnoseSingleEntity(currentShape));
        }
    }
    
    return results;
}

bool STLExportDiagnoser::readStepAsSeparateShapes(const std::string& filename, std::vector<TopoDS_Shape>& outShapes) const
{
    STEPControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(filename.c_str());
    
    if (status != IFSelect_RetDone) {
        std::cerr << "无法读取STEP文件" << std::endl;
        return false;
    }
    
    // 读取文件中的所有独立实体
    reader.TransferRoots();
    
    // 获取模型中的根实体列表
    Handle(TColStd_HSequenceOfTransient) list = reader.GiveList("xst-transferrable-roots");
    
    if (list.IsNull() || list->Length() == 0) {
        std::cerr << "未找到可转换的实体" << std::endl;
        return false;
    }
    
    std::cout << "找到 " << list->Length() << " 个可转换的根实体" << std::endl;
    
    // 遍历并转换每个独立实体
    for (int i = 1; i <= list->Length(); i++) {
        if (reader.TransferEntity(list->Value(i)) == IFSelect_RetDone) {
            TopoDS_Shape shape = reader.Shape(i);
            if (!shape.IsNull()) {
                outShapes.push_back(shape);
                std::cout << "实体 " << i << " 转换成功" << std::endl;
            }
        }
    }
    
    return true;
}

MeshDiagnosis STLExportDiagnoser::diagnoseSingleEntity(const TopoDS_Shape& shape)
{
    MeshDiagnosis diag;
    diag.shape = shape;
    diag.isMeshable = false;
    diag.faceCount = 0;
    diag.triangulatedFaceCount = 0;
    
    try {
        // 尝试对当前独立实体进行网格化
        BRepMesh_IncrementalMesh mesher(shape, myDeflection, false, 0.5, true);
        
        // 诊断：检查该实体中实际有多少个面被成功三角化
        for (TopExp_Explorer faceExplorer(shape, TopAbs_FACE);
             faceExplorer.More();
             faceExplorer.Next()) {
            
            diag.faceCount++;
            TopoDS_Face face = TopoDS::Face(faceExplorer.Current());
            TopLoc_Location location;
            
            // 获取面的三角化结果
            Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);
            
            if (!triangulation.IsNull() && triangulation->NbTriangles() > 0) {
                diag.triangulatedFaceCount++;
            }
        }
        
        // 判断标准：如果大部分面都被成功三角化，则认为该实体可导出
        if (diag.faceCount > 0 && 
            (float)diag.triangulatedFaceCount / diag.faceCount > 0.9) {
            diag.isMeshable = true;
            diag.failureReason = "";
        } else {
            diag.failureReason = "三角化覆盖率不足 (" + 
                                 std::to_string(diag.triangulatedFaceCount) + 
                                 "/" + std::to_string(diag.faceCount) + " faces)";
        }
        
    } catch (Standard_Failure& e) {
        diag.failureReason = "网格化过程异常: " + std::string(e.GetMessageString());
    }
    
    return diag;
}
