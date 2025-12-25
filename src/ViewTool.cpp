#include "ViewTool.h"
#include <iostream>

#include <TopoDS_Shape.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <Poly_Triangulation.hxx>
#include <TopoDS_Face.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <Geom_Surface.hxx>
#include <BRep_Tool.hxx>
#include  <TopLoc_Location.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopoDS.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <Poly_Polygon3D.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepAdaptor_Curve.hxx>

bool ViewTool::isShapeEmpty(const TopoDS_Shape& shape)
{
    static const auto isEveryShapeInCompoundEmpty = [](const TopoDS_Shape& shape) {
        for (TopoDS_Iterator it(shape); it.More(); it.Next()) {
            if (const TopoDS_Shape& sub = it.Value(); !isShapeEmpty(sub)) {
                // Found a non-empty sub-shape
                return false;
            }
        }

        return true;
        };

    // If shape is null we consider it as empty
    if (shape.IsNull()) {
        return true;
    }

    if (shape.ShapeType() == TopAbs_COMPOUND) {
        return isEveryShapeInCompoundEmpty(shape);
    }

    // To see if shape is non-empty we check if it has at least one vertex
    TopExp_Explorer explorer(shape, TopAbs_VERTEX);
    return !explorer.More();
}

Handle(Poly_Polygon3D) ViewTool::polygonOfEdge(const TopoDS_Edge& edge, TopLoc_Location& loc)
{
    BRepAdaptor_Curve adapt(edge);
    double u = adapt.FirstParameter();
    double v = adapt.LastParameter();
    Handle(Poly_Polygon3D) aPoly = BRep_Tool::Polygon3D(edge, loc);
    if (!aPoly.IsNull() && !Precision::IsInfinite(u) && !Precision::IsInfinite(v))
        return aPoly;

    // recreate an edge with a clear range
    u = std::max(-50.0, u);
    v = std::min(50.0, v);

    double uv;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, uv, uv);

    BRepBuilderAPI_MakeEdge mkBuilder(curve, u, v);
    TopoDS_Shape shape = mkBuilder.Shape();
    // why do we have to set the inverted location here?
    TopLoc_Location inv = loc.Inverted();
    shape.Location(inv);

    BRepMesh_IncrementalMesh incrementalMesh(shape, 0.005, false, 0.1, true);
    TopLoc_Location tmp;
    return BRep_Tool::Polygon3D(TopoDS::Edge(shape), tmp);
}

Handle(Poly_Triangulation) ViewTool::triangulationOfFace(const TopoDS_Face& face)
{
    TopLoc_Location loc;
    Handle(Poly_Triangulation) mesh = BRep_Tool::Triangulation(face, loc);
    if (!mesh.IsNull())
        return mesh;

    // If no triangulation exists then the shape is probably infinite
    double u1{}, u2{}, v1{}, v2{};
    try {
        BRepAdaptor_Surface adapt(face);
        u1 = adapt.FirstUParameter();
        u2 = adapt.LastUParameter();
        v1 = adapt.FirstVParameter();
        v2 = adapt.LastVParameter();
    }
    catch (const Standard_Failure&) {
        return nullptr;
    }

    auto selectRange = [](double& p1, double& p2) {
        if (Precision::IsInfinite(p1) && Precision::IsInfinite(p2)) {
            p1 = -50.0;
            p2 = 50.0;
        }
        else if (Precision::IsInfinite(p1)) {
            p1 = p2 - 100.0;
        }
        else if (Precision::IsInfinite(p2)) {
            p2 = p1 + 100.0;
        }
        };

    // recreate a face with a clear boundary in case it's infinite
    selectRange(u1, u2);
    selectRange(v1, v2);

    Handle(Geom_Surface) surface = BRep_Tool::Surface(face);
    if (surface.IsNull()) {
		std::cout << "Cannot create surface from face" << std::endl;
    }
    BRepBuilderAPI_MakeFace mkBuilder(surface, u1, u2, v1, v2, Precision::Confusion());
    TopoDS_Shape shape = mkBuilder.Shape();
    shape.Location(loc);

    BRepMesh_IncrementalMesh meshBuilder(shape, 0.005, false, 0.1, true);
    return BRep_Tool::Triangulation(TopoDS::Face(shape), loc);
}

Bnd_Box ViewTool::getBounds(const TopoDS_Shape& shape)
{
    Bnd_Box bounds;
    BRepBndLib::Add(shape, bounds);
    bounds.SetGap(0.0);

    return bounds;
}

Standard_Real ViewTool::getDeflection(const Bnd_Box& bounds, double deviation)
{
    Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
    bounds.Get(xMin, yMin, zMin, xMax, yMax, zMax);

    // calculating the deflection value
    return ((xMax - xMin) + (yMax - yMin) + (zMax - zMin)) / 300.0 * deviation;
}

Standard_Real ViewTool::getDeflection(const TopoDS_Shape& shape, double deviation)
{
    return getDeflection(getBounds(shape), deviation);
}
