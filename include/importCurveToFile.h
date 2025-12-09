#ifndef IMPORT_CURVE_TO_FILE_H
#define IMPORT_CURVE_TO_FILE_H

#include <TopoDS_Shape.hxx>
#include <Geom_Curve.hxx>
#include <TopoDS_Edge.hxx>


#include <vector>
#include <string>
#include <map>

/**
 * @class importCurveToFile
 * @brief 用于筛选模型中的所有曲线并分类保存到多个BREP文件中
 */
class importCurveToFile {
public:
    /**
     * @brief 构造函数
     */
    importCurveToFile();

    /**
     * @brief 析构函数
     */
    ~importCurveToFile();

    /**
     * @brief 处理模型，提取并分类保存曲线
     * @param shape 输入的模型形状
     * @param outputDir 输出目录，默认为当前目录
     * @return 是否成功
     */
    bool processAndSaveCurves(const TopoDS_Shape& shape, const std::string& outputDir = ".");

    /**
     * @brief 处理模型，提取所有曲线并保存到单个BREP文件
     * @param shape 输入的模型形状
     * @param outputFile 输出BREP文件路径
     * @return 是否成功
     */
    bool processAndSaveAllCurvesToSingleBREP(const TopoDS_Shape& shape, const std::string& outputFile);

    /**
     * @brief 处理模型，提取所有曲线并离散化保存点到文件
     * @param shape 输入的模型形状
     * @param outputFile 输出点文件路径
     * @param deflection 离散化精度，默认为1e-3
     * @return 是否成功
     */
    bool processAndSaveCurvesAsPoints(const TopoDS_Shape& shape, const std::string& outputFile, double deflection = 1e-3);

    /**
     * @brief 获取曲线类型统计
     * @return 曲线类型到数量的映射
     */
    const std::map<std::string, int>& getCurveTypeStats() const;

    /**
     * @brief 设置是否将修剪曲线的基础曲线提取出来
     * @param extract 是否提取，默认为true
     */
    void setExtractBasisCurves(bool extract);

    /**
     * @brief 设置是否保存修剪曲线
     * @param save 是否保存，默认为false
     */
    void setSaveTrimmedCurves(bool save);

private:
    /**
     * @brief 提取形状中的所有曲线
     * @param shape 输入形状
     * @param curveMap 输出曲线映射，键为曲线类型，值为该类型的曲线集合
     */
    void extractCurves(const TopoDS_Shape& shape, std::map<std::string, std::vector<TopoDS_Edge>>& curveMap);

    /**
     * @brief 获取曲线类型名称
     * @param curve 输入曲线
     * @return 曲线类型名称
     */
    std::string getCurveTypeName(const Handle(Geom_Curve)& curve);

    /**
     * @brief 将曲线集合保存到BREP文件
     * @param edges 曲线集合
     * @param filename 文件名
     * @return 是否成功
     */
    bool saveEdgesToBREP(const std::vector<TopoDS_Edge>& edges, const std::string& filename);

private:
    std::map<std::string, int> m_curveTypeStats; ///< 曲线类型统计
    bool m_extractBasisCurves; ///< 是否提取修剪曲线的基础曲线
    bool m_saveTrimmedCurves; ///< 是否保存修剪曲线
};

#endif // IMPORT_CURVE_TO_FILE_H
