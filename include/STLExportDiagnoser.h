#ifndef STLEXPORTDIAGNOSER_H
#define STLEXPORTDIAGNOSER_H

#include <TopoDS_Shape.hxx>
#include <vector>
#include <string>

// 诊断结果结构体
struct MeshDiagnosis {
    TopoDS_Shape shape;
    bool isMeshable;
    std::string failureReason;
    int faceCount;
    int triangulatedFaceCount;
};

class STLExportDiagnoser {
public:
    // 构造函数，设置网格化精度
    explicit STLExportDiagnoser(double aDeflection = 0.01);
    
    // 诊断单个形状
    std::vector<MeshDiagnosis> diagnoseShape(const TopoDS_Shape& aShape);
    
    // 从STEP文件读取并诊断所有形状
    bool diagnoseStepFile(const std::string& filename, std::vector<MeshDiagnosis>& outDiagnoses);
    
    // 获取可导出的形状
    std::vector<TopoDS_Shape> getExportableShapes(const std::vector<MeshDiagnosis>& diagnoses) const;
    
    // 获取不可导出的形状
    std::vector<TopoDS_Shape> getNonExportableShapes(const std::vector<MeshDiagnosis>& diagnoses) const;
    
    // 导出可导出的形状到STL文件
    bool exportToSTL(const std::vector<TopoDS_Shape>& shapes, const std::string& filename) const;
    
    // 保存不可网格化的部分到BREP文件
    bool saveNonMeshablePartsToBREP(const std::vector<MeshDiagnosis>& diagnoses, const std::string& brepFilename) const;
    
    // 生成诊断报告
    std::string generateReport(const std::vector<MeshDiagnosis>& diagnoses) const;
    
    // 设置网格化精度
    void setDeflection(double aDeflection);
    
    // 获取网格化精度
    double getDeflection() const;
    
private:
    // 分解模型并独立诊断
    std::vector<MeshDiagnosis> diagnoseAndMeshShapes(const TopoDS_Shape& aShape);
    
    // 递归分解形状的辅助函数
    void recursiveDiagnose(const TopoDS_Shape& aShape, std::vector<MeshDiagnosis>& results);
    
    // 从STEP文件读取并诊断所有形状
    bool readStepAsSeparateShapes(const std::string& filename, std::vector<TopoDS_Shape>& outShapes) const;
    
    // 诊断单个实体
    MeshDiagnosis diagnoseSingleEntity(const TopoDS_Shape& shape);
    
    // 成员变量
    double myDeflection; // 网格化精度
};

#endif // STLEXPORTDIAGNOSER_H