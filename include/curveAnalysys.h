#pragma once

#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <vector>

// Curve classification result structure
struct CurveClassificationResult {
    std::vector<TopoDS_Edge> isolatedCurves;        // Curves not belonging to any face
    std::vector<TopoDS_Edge> intersectingCurves;    // Curves intersecting with faces
    std::vector<TopoDS_Edge> onSurfaceCurves;       // Curves entirely on surfaces
};

class curveAnalysys {
public:
    curveAnalysys();
    ~curveAnalysys();

    // Analyze the model and classify curves
    CurveClassificationResult analyze(const TopoDS_Shape& model);

private:
    // Helper methods
    bool isEdgeOnFace(const TopoDS_Edge& edge, const TopoDS_Face& face);
    bool doesEdgeIntersectFace(const TopoDS_Edge& edge, const TopoDS_Face& face);
    bool isEdgeIsolated(const TopoDS_Edge& edge, const std::vector<TopoDS_Face>& allFaces);
    
    // Store all faces for analysis
    std::vector<TopoDS_Face> m_allFaces;
};
