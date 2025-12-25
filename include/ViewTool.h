#pragma once
#include <functional>
#include <Standard_Handle.hxx>
#include <TopoDS_Shape.hxx>

class Poly_Triangulation;
class TopoDS_Face;
class Bnd_Box;
class Poly_Polygon3D;
class TopoDS_Edge;
class TopLoc_Location;
namespace Jumpers 
{
	class ShapeMapHasher
	{
	public:
		size_t operator()(const TopoDS_Shape& theShape) const
		{
			return std::hash<TopoDS_Shape>{}(theShape);
		}
	};
}


class ViewTool 
{


public:
	static bool isShapeEmpty(const TopoDS_Shape& shape);

	static Handle(Poly_Polygon3D) polygonOfEdge(const TopoDS_Edge& edge, TopLoc_Location& loc);

	static Handle(Poly_Triangulation) triangulationOfFace(const TopoDS_Face& face);

	static Bnd_Box getBounds(const TopoDS_Shape& shape);

	static Standard_Real getDeflection(const Bnd_Box& bounds, double deviation);

	static Standard_Real getDeflection(const TopoDS_Shape& shape, double deviation);
};
