#pragma once

#include <TopTools_ListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <ShapeFix_Shape.hxx>
#include <TopoDS_Shape.hxx>
#include <STEPControl_Reader.hxx>

class OcctUtil
{
public:
    OcctUtil();
    ~OcctUtil();

    // Separate by solids and shells
    static void SeparateBySolidsAndShells(const TopoDS_Shape& model, TopoDS_Compound& meshableParts, TopoDS_Compound& nonMeshableParts, double deflection = 0.01);

};