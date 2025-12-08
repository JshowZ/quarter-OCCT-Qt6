#ifndef COMPOUNDCURVEEXTRACTOR_H
#define COMPOUNDCURVEEXTRACTOR_H

#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <string>

class CompoundCurveExtractor {
public:
    CompoundCurveExtractor();
    ~CompoundCurveExtractor();
    
    // 从复合形状中提取所有曲线
    bool extractCurvesFromCompound(const TopoDS_Shape& inputShape);
    
    // 将提取的曲线保存到BREP文件
    bool saveCurvesToBREP(const std::string& filePath) const;
    
    // 获取提取的曲线列表
    const TopTools_ListOfShape& getExtractedCurves() const;
    
    // 获取提取的曲线数量
    int getCurveCount() const;
    
    // 重置提取结果
    void reset();
    
    // 从文件中加载形状并提取曲线
    bool loadAndExtractCurves(const std::string& stepFilePath);
    
private:
    // 递归遍历形状并提取曲线
    void traverseShape(const TopoDS_Shape& shape);
    
    // 从单个复合形状中提取曲线
    void extractCurvesFromCompoundRecursive(const TopoDS_Shape& compoundShape);
    
    // 检查形状是否为复合形状
    bool isCompound(const TopoDS_Shape& shape) const;
    
    // 成员变量
    TopTools_ListOfShape m_extractedCurves;
};

#endif // COMPOUNDCURVEEXTRACTOR_H