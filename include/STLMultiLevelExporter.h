#ifndef STLMULTILEVELEXPORTER_H
#define STLMULTILEVELEXPORTER_H

#include <TopoDS_Shape.hxx>
#include <vector>
#include <string>

struct ExportResult {
    TopoDS_Shape shape;
    std::string shapeLevel;
    bool exportedSuccessfully;
    int faceCount;
    int triangulatedFaceCount;
    std::string failureReason;
};

class STLMultiLevelExporter {
public:
    explicit STLMultiLevelExporter(double deflection = 0.01);
    
    // 多级分解并导出模型到STL
    bool exportToSTL(const TopoDS_Shape& inputShape, const std::string& filename);
    
    // 多级分解模型，返回分解结果
    std::vector<ExportResult> decomposeAndAnalyze(const TopoDS_Shape& inputShape);
    
    // 设置网格化精度
    void setDeflection(double deflection);
    
    // 获取网格化精度
    double getDeflection() const;
    
    // 获取导出结果
    const std::vector<ExportResult>& getExportResults() const;
    
    // 生成导出报告
    std::string generateReport() const;
    
    // 获取成功导出的形状数量
    int getSuccessCount() const;
    
    // 获取失败导出的形状数量
    int getFailureCount() const;
    
    // 获取总形状数量
    int getTotalShapeCount() const;
    
private:
    // 递归分解形状
    void decomposeShape(const TopoDS_Shape& shape, const std::string& currentLevel, std::vector<ExportResult>& results);
    
    // 尝试导出单个形状到STL
    bool tryExportShape(const TopoDS_Shape& shape, ExportResult& result);
    
    // 检查形状是否可三角化
    bool isTriangulable(const TopoDS_Shape& shape);
    
    // 计算形状的三角化覆盖率
    double calculateTriangulationCoverage(const TopoDS_Shape& shape);
    
    // 成员变量
    double m_deflection;
    std::vector<ExportResult> m_exportResults;
    
    // 常量定义
    static const double TRIANGULATION_COVERAGE_THRESHOLD;
};

#endif // STLMULTILEVELEXPORTER_H