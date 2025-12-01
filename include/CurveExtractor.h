#pragma once

#include <TopoDS_Shape.hxx>
#include <BRep_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_BSplineCurve.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <TopoDS.hxx>
#include <gp_Ax2.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>

class CurveExtractor {
public:
    static void analyzeCurves(const TopoDS_Shape& shape);

private:
    static void analyzeSingleCurve(const TopoDS_Edge& edge, int index);
};