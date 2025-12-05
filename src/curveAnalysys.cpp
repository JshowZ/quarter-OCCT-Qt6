#include "curveAnalysys.h"
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <Precision.hxx>

curveAnalysys::curveAnalysys()
{
}

curveAnalysys::~curveAnalysys()
{
}

CurveClassificationResult curveAnalysys::analyze(const TopoDS_Shape& model)
{
    CurveClassificationResult result;
    
    // Clear previous results
    result.isolatedCurves.clear();
    result.intersectingCurves.clear();
    result.onSurfaceCurves.clear();
    
    // Collect all faces first
    m_allFaces.clear();
    TopExp_Explorer faceExp(model, TopAbs_FACE);
    while (faceExp.More())
    {
        m_allFaces.push_back(TopoDS::Face(faceExp.Current()));
        faceExp.Next();
    }
    
    // Iterate through all edges (curves)
    TopExp_Explorer edgeExp(model, TopAbs_EDGE);
    while (edgeExp.More())
    {
        const TopoDS_Edge& edge = TopoDS::Edge(edgeExp.Current());
        
        bool isOnSurface = false;
        bool intersectsFace = false;
        
        // 1. Check if the edge is entirely on any face
        for (const auto& face : m_allFaces)
        {
            if (isEdgeOnFace(edge, face))
            {
                isOnSurface = true;
                break; // No need to check other faces
            }
        }
        
        if (isOnSurface)
        {
            result.onSurfaceCurves.push_back(edge);
            continue; // Move to next edge
        }
        
        // 2. If not on any surface, check if it intersects any face
        for (const auto& face : m_allFaces)
        {
            if (doesEdgeIntersectFace(edge, face))
            {
                intersectsFace = true;
                break; // No need to check other faces
            }
        }
        
        if (intersectsFace)
        {
            result.intersectingCurves.push_back(edge);
            continue; // Move to next edge
        }
        
        // 3. If neither on surface nor intersecting any face, it's isolated
        result.isolatedCurves.push_back(edge);
        
        edgeExp.Next();
    }
    
    return result;
}

bool curveAnalysys::isEdgeOnFace(const TopoDS_Edge& edge, const TopoDS_Face& face)
{
    try
    {
        // First, quick check: is the edge part of the face's topology?
        TopExp_Explorer faceEdgeExp(face, TopAbs_EDGE);
        while (faceEdgeExp.More())
        {
            const TopoDS_Edge& faceEdge = TopoDS::Edge(faceEdgeExp.Current());
            if (faceEdge.IsSame(edge))
            {
                return true;
            }
            faceEdgeExp.Next();
        }
        
        // Geometric check: calculate distance and sample points
        BRepExtrema_DistShapeShape extrema(edge, face);
        extrema.Perform();
        
        if (!extrema.IsDone())
        {
            return false;
        }
        
        // Check minimum distance is within tolerance
        double minDistance = extrema.Value();
        const double tolerance = 1e-6;
        if (minDistance > tolerance)
        {
            return false;
        }
        
        // Sample points along the curve to ensure it's entirely on the face
        Standard_Real first, last;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
        
        if (curve.IsNull())
        {
            return false;
        }
        
        // Get the face's surface
        Handle(Geom_Surface) surface = BRep_Tool::Surface(face);
        if (surface.IsNull())
        {
            return false;
        }
        
        // Sample 5 points along the curve
        for (int i = 0; i <= 4; ++i)
        {
            double param = first + i * (last - first) / 4.0;
            gp_Pnt curvePoint = curve->Value(param);
            
            // Check if this point is on the face by projecting onto the surface
            // and checking the distance
            gp_Pnt2d uv;
            gp_Pnt projPoint;
            
            // Project point onto surface
            GeomAPI_ProjectPointOnSurf projector(curvePoint, surface);
            if (projector.NbPoints() == 0)
            {
                return false; // No projection found
            }
            
            projPoint = projector.NearestPoint();
            double distance = curvePoint.Distance(projPoint);
            
            if (distance > tolerance)
            {
                return false;
            }
        }
        
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool curveAnalysys::doesEdgeIntersectFace(const TopoDS_Edge& edge, const TopoDS_Face& face)
{
    try
    {
        // Use BRepExtrema to check if edge intersects face
        BRepExtrema_DistShapeShape extrema(edge, face);
        extrema.Perform();
        
        if (!extrema.IsDone())
        {
            return false;
        }
        
        // If the minimum distance is zero (within precision), consider it intersecting
        return extrema.Value() < Precision::Confusion();
    }
    catch (...)
    {
        return false;
    }
}

bool curveAnalysys::isEdgeIsolated(const TopoDS_Edge& edge, const std::vector<TopoDS_Face>& allFaces)
{
    // An edge is isolated if it's not on any face and doesn't intersect any face
    for (const auto& face : allFaces)
    {
        if (isEdgeOnFace(edge, face) || doesEdgeIntersectFace(edge, face))
        {
            return false;
        }
    }
    return true;
}
