#include "CurveExtractor.h"

void CurveExtractor::analyzeCurves(const TopoDS_Shape& shape)
{

	std::cout << "=== Curves Analysis ===" << std::endl;

	int edgeCount = 0;
	TopExp_Explorer explorer(shape, TopAbs_EDGE);

	while (explorer.More()) {
		const TopoDS_Edge& edge = TopoDS::Edge(explorer.Current());
		analyzeSingleCurve(edge, ++edgeCount);
		explorer.Next();
	}

	std::cout << "count: " << edgeCount << " edges" << std::endl;

}

void CurveExtractor::analyzeSingleCurve(const TopoDS_Edge& edge, int index)
{
    Standard_Real first, last;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);

    if (curve.IsNull()) {
        std::cout << "edge " << index << ": no curve" << std::endl;
        return;
    }

    std::cout << "edge " << index << ": ";
    std::cout << "range [" << first << ", " << last << "] - ";

    if (curve->IsKind(STANDARD_TYPE(Geom_Line))) {
        Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(curve);
        gp_Dir dir = line->Position().Direction();
        std::cout << "line, direction("
            << dir.X() << ", " << dir.Y() << ", " << dir.Z() << ")";
    }
    else if (curve->IsKind(STANDARD_TYPE(Geom_Circle))) {
        Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(curve);
        Standard_Real radius = circle->Radius();
        gp_Pnt center = circle->Location();
        std::cout << "radius" << radius
            << "center.X()" << center.X() << "center.Y() "
            << center.Y() << " center.Z()" << center.Z() << ")";
    }
    else if (curve->IsKind(STANDARD_TYPE(Geom_Ellipse))) {
        Handle(Geom_Ellipse) ellipse = Handle(Geom_Ellipse)::DownCast(curve);
        std::cout << "ellipse->MajorRadius()" << ellipse->MajorRadius()
            << "ellipse->MinorRadius()" << ellipse->MinorRadius();
    }
    else if (curve->IsKind(STANDARD_TYPE(Geom_BSplineCurve))) {
        Handle(Geom_BSplineCurve) bspline = Handle(Geom_BSplineCurve)::DownCast(curve);
        std::cout << "bspline->Degree()" << bspline->Degree()
            << "bspline->NbPoles()" << bspline->NbPoles();
    }
    else {
        std::cout << "unknow: " << curve->DynamicType()->Name();
    }

    std::cout << std::endl;
}
