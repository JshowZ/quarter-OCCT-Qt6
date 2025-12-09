#include "importCurveToFile.h"

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
#include <Geom_BoundedCurve.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <BRepTools.hxx>
#include <Standard_Failure.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <gp_Pnt.hxx>

#include <iostream>
#include <fstream>

importCurveToFile::importCurveToFile()
    : m_extractBasisCurves(true),
      m_saveTrimmedCurves(false) {
}

importCurveToFile::~importCurveToFile() {
}

bool importCurveToFile::processAndSaveCurves(const TopoDS_Shape& shape, const std::string& outputDir) {
    try {
        // 清空之前的统计数据
        m_curveTypeStats.clear();

        // 提取并分类曲线
        std::map<std::string, std::vector<TopoDS_Edge>> curveMap;
        extractCurves(shape, curveMap);

        // 为每种曲线类型创建BREP文件
        for (const auto& pair : curveMap) {
            const std::string& curveType = pair.first;
            const std::vector<TopoDS_Edge>& edges = pair.second;

            if (edges.empty()) {
                continue;
            }

            // 构建输出文件名
            std::string filename = outputDir + "/" + curveType + "_curves.brep";

            // 保存到BREP文件
            if (!saveEdgesToBREP(edges, filename)) {
                std::cerr << "Failed to save " << curveType << " curves to " << filename << std::endl;
                return false;
            }

            // 更新统计信息
            m_curveTypeStats[curveType] = edges.size();

            std::cout << "Saved " << edges.size() << " " << curveType << " curves to " << filename << std::endl;
        }

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

const std::map<std::string, int>& importCurveToFile::getCurveTypeStats() const {
    return m_curveTypeStats;
}

void importCurveToFile::setExtractBasisCurves(bool extract) {
    m_extractBasisCurves = extract;
}

void importCurveToFile::setSaveTrimmedCurves(bool save) {
    m_saveTrimmedCurves = save;
}

void importCurveToFile::extractCurves(const TopoDS_Shape& shape, std::map<std::string, std::vector<TopoDS_Edge>>& curveMap) {
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
            if (m_saveTrimmedCurves) {
                // 保存修剪曲线
                std::string curveType = getCurveTypeName(curve);
                curveMap[curveType].push_back(edge);
            }

            if (m_extractBasisCurves) {
                // 提取基础曲线
                Handle(Geom_Curve) basisCurve = trimmedCurve->BasisCurve();
                std::string basisType = getCurveTypeName(basisCurve);

                // 为基础曲线创建新的边
                BRepBuilderAPI_MakeEdge edgeMaker(basisCurve);
                if (edgeMaker.IsDone()) {
                    TopoDS_Edge newEdge = edgeMaker.Edge();
                    curveMap[basisType].push_back(newEdge);
                }
            }
        } else {
            // 直接处理非修剪曲线
            std::string curveType = getCurveTypeName(curve);
            curveMap[curveType].push_back(edge);
        }
    }
}

std::string importCurveToFile::getCurveTypeName(const Handle(Geom_Curve)& curve) {
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

bool importCurveToFile::saveEdgesToBREP(const std::vector<TopoDS_Edge>& edges, const std::string& filename) {
    if (edges.empty()) {
        return false;
    }

    // 创建复合形状，用于保存所有边
    BRep_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);

    // 将所有边添加到复合形状中
    for (const auto& edge : edges) {
        builder.Add(compound, edge);
    }

    // 保存到BREP文件
    return BRepTools::Write(compound, filename.c_str());
}

bool importCurveToFile::processAndSaveAllCurvesToSingleBREP(const TopoDS_Shape& shape, const std::string& outputFile) {
    try {
        // 清空之前的统计数据
        m_curveTypeStats.clear();

        // 提取所有曲线
        std::map<std::string, std::vector<TopoDS_Edge>> curveMap;
        extractCurves(shape, curveMap);

        // 收集所有曲线到一个向量中
        std::vector<TopoDS_Edge> allEdges;
        for (const auto& pair : curveMap) {
            const std::vector<TopoDS_Edge>& edges = pair.second;
            allEdges.insert(allEdges.end(), edges.begin(), edges.end());
            
            // 更新统计信息
            m_curveTypeStats[pair.first] = edges.size();
        }

        // 保存所有曲线到单个BREP文件
        return saveEdgesToBREP(allEdges, outputFile);
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
