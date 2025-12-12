#include "STLExportWithCurvePoints.h"

#include <StlAPI_Writer.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Parabola.hxx>
#include <Geom_Hyperbola.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GCPnts_UniformDeflection.hxx>
#include <gp_Pnt.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <Standard_Failure.hxx>
#include <iostream>
#include <fstream>

STLExportWithCurvePoints::STLExportWithCurvePoints()
    : m_stlDeflection(0.01),
      m_curveDeflection(1e-3),
      m_exportedShapeCount(0) {
}

STLExportWithCurvePoints::~STLExportWithCurvePoints() {
}

void STLExportWithCurvePoints::setSTLDeflection(double deflection) {
    m_stlDeflection = deflection;
}

void STLExportWithCurvePoints::setCurveDeflection(double deflection) {
    m_curveDeflection = deflection;
}

bool STLExportWithCurvePoints::exportSTLAndExtractCurves(const TopoDS_Shape& shape, 
                                                       const std::string& stlFilename, 
                                                       std::vector<CurvePointSet>& curvePointSets) {
    try {
        // 清空之前的统计数据
        m_curveTypeStats.clear();
        m_exportedShapeCount = 0;

        // 第一步：将模型导出为STL文件
        // 1.1 创建网格化器
        BRepMesh_IncrementalMesh mesher(shape, m_stlDeflection, false, 0.5, true);
        mesher.Perform();
        
        if (!mesher.IsDone()) {
            std::cerr << "Failed to mesh the shape for STL export." << std::endl;
            return false;
        }
        
        // 1.2 导出为STL
        StlAPI_Writer stlWriter;
        bool stlSuccess = stlWriter.Write(shape, stlFilename.c_str());
        
        if (!stlSuccess) {
            std::cerr << "Failed to export shape to STL file: " << stlFilename << std::endl;
            return false;
        }
        
        m_exportedShapeCount = 1;
        
        // 第二步：提取所有曲线并离散化为点集
        std::map<std::string, std::vector<TopoDS_Edge>> curveMap;
        extractCurves(shape, curveMap);
        
        // 将曲线离散化为点集并存储到数据结构中
        discretizeCurves(curveMap, curvePointSets);
        
        return true;
    } catch (const Standard_Failure& e) {
        std::cerr << "OCCT Exception: " << e.GetMessageString() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        return false;
    }
}

bool STLExportWithCurvePoints::extractCurvesAsPoints(const TopoDS_Shape& shape, 
                                                  std::vector<CurvePointSet>& curvePointSets) {
    try {
        // 清空之前的统计数据
        m_curveTypeStats.clear();
        
        // 提取所有曲线并离散化为点集
        std::map<std::string, std::vector<TopoDS_Edge>> curveMap;
        extractCurves(shape, curveMap);
        
        // 将曲线离散化为点集并存储到数据结构中
        discretizeCurves(curveMap, curvePointSets);
        
        return true;
    } catch (const Standard_Failure& e) {
        std::cerr << "OCCT Exception: " << e.GetMessageString() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        return false;
    }
}

const std::map<std::string, int>& STLExportWithCurvePoints::getCurveTypeStats() const {
    return m_curveTypeStats;
}

int STLExportWithCurvePoints::getExportedShapeCount() const {
    return m_exportedShapeCount;
}

void STLExportWithCurvePoints::extractCurves(const TopoDS_Shape& shape, 
                                            std::map<std::string, std::vector<TopoDS_Edge>>& curveMap) {
    // 遍历所有边
    TopExp_Explorer edgeExplorer(shape, TopAbs_EDGE);
    for (; edgeExplorer.More(); edgeExplorer.Next()) {
        const TopoDS_Edge& edge = TopoDS::Edge(edgeExplorer.Current());

        Standard_Real first, last;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
        
        if (curve.IsNull()) {
            continue;
        }
        
        // 处理修剪曲线
        Handle(Geom_TrimmedCurve) trimmedCurve = Handle(Geom_TrimmedCurve)::DownCast(curve);
        if (!trimmedCurve.IsNull()) {
            // 获取修剪曲线的基础曲线
            Handle(Geom_Curve) basisCurve = trimmedCurve->BasisCurve();
            std::string curveType = getCurveTypeName(basisCurve);
            curveMap[curveType].push_back(edge);
        } else {
            // 直接处理非修剪曲线
            std::string curveType = getCurveTypeName(curve);
            curveMap[curveType].push_back(edge);
        }
    }
    
    // 更新曲线类型统计
    for (const auto& pair : curveMap) {
        m_curveTypeStats[pair.first] = pair.second.size();
    }
}

void STLExportWithCurvePoints::discretizeCurves(const std::map<std::string, std::vector<TopoDS_Edge>>& curveMap, 
                                               std::vector<CurvePointSet>& curvePointSets) {
    // 清空输出向量
    curvePointSets.clear();
    
    // 遍历每种曲线类型
    for (const auto& pair : curveMap) {
        const std::string& curveType = pair.first;
        const std::vector<TopoDS_Edge>& edges = pair.second;
        
        // 对每条边进行处理
        for (const auto& edge : edges) {
            Standard_Real first, last;
            Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
            
            if (curve.IsNull()) {
                continue;
            }
            
            // 创建曲线适配器
            GeomAdaptor_Curve adaptor(curve);
            
            // 使用UniformDeflection算法进行离散化
            GCPnts_UniformDeflection uniformDeflection(adaptor, m_curveDeflection, first, last);
            
            if (uniformDeflection.IsDone()) {
                // 创建CurvePointSet对象
                CurvePointSet curvePointSet;
                curvePointSet.curveType = curveType;
                
                // 获取离散点并添加到CurvePointSet中
                int pointCount = uniformDeflection.NbPoints();
                for (int i = 1; i <= pointCount; ++i) {
                    gp_Pnt point = uniformDeflection.Value(i);
                    curvePointSet.points.push_back(point);
                }
                
                // 将CurvePointSet添加到输出向量中
                curvePointSets.push_back(curvePointSet);
            }
        }
    }
    
    std::cout << "Successfully discretized " << curvePointSets.size() << " curves into points." << std::endl;
}

std::string STLExportWithCurvePoints::getCurveTypeName(const Handle(Geom_Curve)& curve) {
    if (curve.IsNull()) {
        return "UnknownCurve";
    }

    if (curve->DynamicType() == STANDARD_TYPE(Geom_Line)) {
        return "Line";
    } else if (curve->DynamicType() == STANDARD_TYPE(Geom_Circle)) {
        return "Circle";
    } else if (curve->DynamicType() == STANDARD_TYPE(Geom_Ellipse)) {
        return "Ellipse";
    } else if (curve->DynamicType() == STANDARD_TYPE(Geom_Parabola)) {
        return "Parabola";
    } else if (curve->DynamicType() == STANDARD_TYPE(Geom_Hyperbola)) {
        return "Hyperbola";
    } else if (curve->DynamicType() == STANDARD_TYPE(Geom_BSplineCurve)) {
        return "BSplineCurve";
    } else if (curve->DynamicType() == STANDARD_TYPE(Geom_BezierCurve)) {
        return "BezierCurve";
    } else if (curve->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve)) {
        return "TrimmedCurve";
    } else {
        return "OtherCurve";
    }
}
