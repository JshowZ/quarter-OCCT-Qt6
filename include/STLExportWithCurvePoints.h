#ifndef STL_EXPORT_WITH_CURVE_POINTS_H
#define STL_EXPORT_WITH_CURVE_POINTS_H

#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <Geom_Curve.hxx>

#include <string>
#include <map>
#include <vector>
#include <gp_Pnt.hxx>

/**
 * @struct CurvePointSet
 * @brief 存储单条曲线的离散点集信息
 */
struct CurvePointSet {
    std::string curveType; ///< 曲线类型名称
    std::vector<gp_Pnt> points; ///< 离散化的点集
};

/**
 * @class STLExportWithCurvePoints
 * @brief 用于将模型导出为STL文件并同时提取所有曲线的离散点
 */
class STLExportWithCurvePoints {
public:
    /**
     * @brief 构造函数
     */
    STLExportWithCurvePoints();

    /**
     * @brief 析构函数
     */
    ~STLExportWithCurvePoints();

    /**
     * @brief 设置STL导出的偏转精度
     * @param deflection 偏转精度，默认为0.01
     */
    void setSTLDeflection(double deflection);

    /**
     * @brief 设置曲线离散化的偏转精度
     * @param deflection 偏转精度，默认为1e-3
     */
    void setCurveDeflection(double deflection);

    /**
     * @brief 执行模型导出为STL文件并提取曲线离散点
     * @param shape 输入的模型形状
     * @param stlFilename 输出的STL文件名
     * @param curvePointSets 输出参数，用于存储提取的曲线点集
     * @return 是否成功
     */
    bool exportSTLAndExtractCurves(const TopoDS_Shape& shape, 
                                   const std::string& stlFilename, 
                                   std::vector<CurvePointSet>& curvePointSets);

    /**
     * @brief 仅提取曲线离散点，不导出STL
     * @param shape 输入的模型形状
     * @param curvePointSets 输出参数，用于存储提取的曲线点集
     * @return 是否成功
     */
    bool extractCurvesAsPoints(const TopoDS_Shape& shape, 
                              std::vector<CurvePointSet>& curvePointSets);

    /**
     * @brief 获取曲线类型统计
     * @return 曲线类型到数量的映射
     */
    const std::map<std::string, int>& getCurveTypeStats() const;

    /**
     * @brief 获取STL导出的形状数量
     * @return 导出的形状数量
     */
    int getExportedShapeCount() const;

private:
    /**
     * @brief 提取形状中的所有曲线
     * @param shape 输入形状
     * @param curveMap 输出曲线映射，键为曲线类型，值为该类型的曲线集合
     */
    void extractCurves(const TopoDS_Shape& shape, 
                      std::map<std::string, std::vector<TopoDS_Edge>>& curveMap);

    /**
     * @brief 将曲线离散化并存储到数据结构中
     * @param curveMap 曲线映射，键为曲线类型，值为该类型的曲线集合
     * @param curvePointSets 输出参数，用于存储提取的曲线点集
     */
    void discretizeCurves(const std::map<std::string, std::vector<TopoDS_Edge>>& curveMap, 
                         std::vector<CurvePointSet>& curvePointSets);

    /**
     * @brief 获取曲线类型名称
     * @param curve 输入曲线
     * @return 曲线类型名称
     */
    std::string getCurveTypeName(const Handle(Geom_Curve)& curve);

private:
    double m_stlDeflection; ///< STL导出的偏转精度
    double m_curveDeflection; ///< 曲线离散化的偏转精度
    std::map<std::string, int> m_curveTypeStats; ///< 曲线类型统计
    int m_exportedShapeCount; ///< 导出的形状数量
};

#endif // STL_EXPORT_WITH_CURVE_POINTS_H
