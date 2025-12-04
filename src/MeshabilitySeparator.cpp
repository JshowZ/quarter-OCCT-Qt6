#include "MeshabilitySeparator.h"

// OCCT includes for meshing and geometry
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>

// OCCT includes for topology exploration
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>

// OCCT includes for data structures
#include <TopTools_MapOfShape.hxx>
#include <BRep_Builder.hxx>

// OCCT includes for shape validation and repair
#include <BRepCheck_Analyzer.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeFix_Face.hxx>

// OCCT includes for geometry analysis
#include <Geom_Surface.hxx>
#include <Geom_Curve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <GeomAbs_CurveType.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <GCPnts_AbscissaPoint.hxx>

// OCCT includes for precision and parallel processing
#include <Precision.hxx>
#include <OSD_Parallel.hxx>

#include <TopoDS_Edge.hxx>

// Standard library includes
#include <sstream>
#include <iostream>
#include <iomanip>


std::string StatusToString(BRepCheck_Status theStatus) {
    switch (theStatus) {
    case BRepCheck_NoError:               return "NoError";
    case BRepCheck_InvalidPointOnCurve:   return "InvalidPointOnCurve";
    case BRepCheck_InvalidPointOnCurveOnSurface: return "InvalidPointOnCurveOnSurface";
    case BRepCheck_InvalidPointOnSurface: return "InvalidPointOnSurface";
    case BRepCheck_No3DCurve:             return "No3DCurve";
    case BRepCheck_Multiple3DCurve:       return "Multiple3DCurve";
    case BRepCheck_Invalid3DCurve:        return "Invalid3DCurve";
    case BRepCheck_NoCurveOnSurface:      return "NoCurveOnSurface";
    case BRepCheck_InvalidCurveOnSurface: return "InvalidCurveOnSurface";
    case BRepCheck_InvalidCurveOnClosedSurface: return "InvalidCurveOnClosedSurface";
    case BRepCheck_InvalidSameRangeFlag:  return "InvalidSameRangeFlag";
    case BRepCheck_InvalidSameParameterFlag: return "InvalidSameParameterFlag";
    case BRepCheck_InvalidDegeneratedFlag: return "InvalidDegeneratedFlag";
    case BRepCheck_FreeEdge:              return "FreeEdge";
    case BRepCheck_InvalidMultiConnexity: return "InvalidMultiConnexity";
    case BRepCheck_InvalidRange:          return "InvalidRange";
    case BRepCheck_EmptyWire:             return "EmptyWire";
    case BRepCheck_RedundantEdge:         return "RedundantEdge";
    case BRepCheck_SelfIntersectingWire:  return "SelfIntersectingWire";
    case BRepCheck_NoSurface:             return "NoSurface";
    case BRepCheck_InvalidWire:           return "InvalidWire";
    case BRepCheck_RedundantWire:         return "RedundantWire";
    case BRepCheck_IntersectingWires:     return "IntersectingWires";
    case BRepCheck_InvalidImbricationOfShells: return "InvalidImbricationOfShells";
    case BRepCheck_BadOrientationOfSubshape: return "BadOrientationOfSubshape";
    case BRepCheck_BadOrientation:        return "BadOrientation";
    case BRepCheck_SubshapeNotInShape:    return "SubshapeNotInShape";
    case BRepCheck_InvalidPolygonOnTriangulation: return "InvalidPolygonOnTriangulation";
    case BRepCheck_InvalidToleranceValue: return "InvalidToleranceValue";
    case BRepCheck_EnclosedRegion:        return "EnclosedRegion";
    default:                              return "UnknownStatus";
    }
}

// ========== Constructor Implementation ==========

MeshabilitySeparator::MeshabilitySeparator(
    double deflection,
    double angle,
    bool relative,
    bool parallel
) : deflection_(deflection),
angle_(angle),
relative_(relative),
parallel_(parallel&& OSD_Parallel::NbLogicalProcessors() > 1),
tryFixBeforeMeshing_(true),
useCache_(true)
{
    // Initialize statistics
    ResetStatistics();

    // Initialize output compounds
    BRep_Builder builder;
    builder.MakeCompound(meshableParts_);
    builder.MakeCompound(nonMeshableParts_);
}

// ========== Main Public Method Implementation ==========

bool MeshabilitySeparator::Separate(
    const TopoDS_Shape& inputShape,
    TopoDS_Compound& meshableParts,
    TopoDS_Compound& nonMeshableParts
) {
    // Clear previous results
    meshableInfo_.clear();
    nonMeshableInfo_.clear();
    ClearCache();
    ResetStatistics();

    // Reinitialize output compounds
    BRep_Builder builder;
    builder.MakeCompound(meshableParts_);
    builder.MakeCompound(nonMeshableParts_);

    // Validate input
    if (inputShape.IsNull()) {
        std::cerr << "Error: Input shape is null!" << std::endl;
        return false;
    }

    std::cout << "Starting meshability separation..." << std::endl;
    std::cout << "Input shape type: " << ShapeTypeToString(inputShape.ShapeType()) << std::endl;

    // Start recursive processing
    bool processingSuccess = ProcessShape(inputShape, true);

    // Set output references
    meshableParts = meshableParts_;
    nonMeshableParts = nonMeshableParts_;

    std::cout << "Separation completed!" << std::endl;
    std::cout << "Meshable parts: " << meshableInfo_.size() << " elements" << std::endl;
    std::cout << "Non-meshable parts: " << nonMeshableInfo_.size() << " elements" << std::endl;

    return processingSuccess;
}

// ========== Core Recursive Processing Method ==========

bool MeshabilitySeparator::ProcessShape(const TopoDS_Shape& shape, bool topLevel) {
    if (shape.IsNull()) {
        return false;
    }

    // Check cache first (if enabled)
    bool cachedIsMeshable = false;
    MeshFailureReason cachedReason = SUCCESS;
    if (useCache_ && IsShapeInCache(shape, cachedIsMeshable, cachedReason)) {
        // Found in cache - use cached result
        ShapeAnalysisInfo info = AnalyzeShape(shape);
        info.failureReason = cachedReason;

        BRep_Builder builder;
        if (cachedIsMeshable) {
            builder.Add(meshableParts_, shape);
            meshableInfo_.push_back(info);
        }
        else {
            builder.Add(nonMeshableParts_, shape);
            nonMeshableInfo_.push_back(info);
        }

        UpdateStatistics(cachedReason, shape.ShapeType());
        return cachedIsMeshable;
    }

    // Not in cache - process based on shape type
    TopAbs_ShapeEnum shapeType = shape.ShapeType();
    ShapeAnalysisInfo info = AnalyzeShape(shape);
    bool isMeshable = false;

    switch (shapeType) {
    case TopAbs_COMPOUND:
    case TopAbs_COMPSOLID:
        // Decompose compounds into their components
        DecomposeCompound(shape);
        isMeshable = true; // Compound itself is considered meshable (its components are processed separately)
        info.failureReason = SUCCESS;
        break;

    case TopAbs_SOLID:
        // Try to mesh the solid as a whole
        if (TryMeshing(shape, info)) {
            isMeshable = true;
        }
        else {
            // Solid meshing failed - decompose into shells
            DecomposeSolid(TopoDS::Solid(shape));
            isMeshable = false; // Solid itself failed meshing
        }
        break;

    case TopAbs_SHELL:
        // Try to mesh the shell as a whole
        if (TryMeshing(shape, info)) {
            isMeshable = true;
        }
        else {
            // Shell meshing failed - decompose into faces
            DecomposeShell(TopoDS::Shell(shape));
            isMeshable = false; // Shell itself failed meshing
        }
        break;

    case TopAbs_FACE:
        // Face is the atomic unit - process it directly
        ProcessFace(TopoDS::Face(shape));
        return true; // Processing handled in ProcessFace

    case TopAbs_WIRE:
    case TopAbs_EDGE:
        // Wires and edges cannot be meshed to STL
        ProcessEdge(shape);
        isMeshable = false;
        info.failureReason = UNSUPPORTED_SURFACE;
        break;

    case TopAbs_VERTEX:
        // Vertices cannot be meshed to STL
        ProcessVertex(shape);
        isMeshable = false;
        info.failureReason = UNSUPPORTED_SURFACE;
        break;

    default:
        // Unknown shape type
        isMeshable = false;
        info.failureReason = OTHER_REASON;
        info.reasonDescription = "Unknown shape type";
        break;
    }

    // For shapes that weren't processed by decomposition methods
    if (shapeType != TopAbs_FACE && shapeType != TopAbs_EDGE && shapeType != TopAbs_VERTEX) {
        BRep_Builder builder;

        if (isMeshable && shapeType != TopAbs_COMPOUND && shapeType != TopAbs_COMPSOLID) {
            // Add meshable solids/shells to output
            builder.Add(meshableParts_, shape);
            meshableInfo_.push_back(info);
        }
        else if (!isMeshable && (shapeType == TopAbs_SOLID || shapeType == TopAbs_SHELL)) {
            // Add failed solids/shells to non-meshable output
            builder.Add(nonMeshableParts_, shape);
            nonMeshableInfo_.push_back(info);
        }
        // Compounds are not added to output - only their components are

        // Cache the result
        if (useCache_) {
            AddToCache(shape, isMeshable, info.failureReason);
        }
    }

    UpdateStatistics(info.failureReason, shapeType);
    return isMeshable;
}

// ========== Meshing Attempt Method ==========

bool MeshabilitySeparator::TryMeshing(const TopoDS_Shape& shape, ShapeAnalysisInfo& info) {
    if (shape.IsNull()) {
        info.failureReason = NULL_GEOMETRY;
        info.reasonDescription = "Shape is null";
        return false;
    }

    // Optional geometry repair
    TopoDS_Shape shapeToMesh = shape;
    if (tryFixBeforeMeshing_) {
        shapeToMesh = TryFixShape(shape);
    }

    try {
        // Attempt meshing
        BRepMesh_IncrementalMesh mesher(shapeToMesh, deflection_, angle_, relative_, parallel_);
        mesher.Perform();

        if (!mesher.IsDone()) {
            info.failureReason = MESHING_FAILED;
            info.reasonDescription = "Meshing algorithm failed to complete";
            return false;
        }

        // Verify that triangles were actually generated
        int totalTriangles = 0;
        int faceCount = 0;

        for (TopExp_Explorer faceExp(shapeToMesh, TopAbs_FACE); faceExp.More(); faceExp.Next()) {
            TopLoc_Location loc;
            Handle(Poly_Triangulation) triangulation =
                BRep_Tool::Triangulation(TopoDS::Face(faceExp.Current()), loc);

            if (triangulation.IsNull()) {
                info.failureReason = NO_TRIANGLES;
                info.reasonDescription = "Face generated null triangulation";
                return false;
            }

            totalTriangles += triangulation->NbTriangles();
            faceCount++;
        }

        // Update analysis info
        info.triangleCount = totalTriangles;
        info.faceCount = faceCount;

        // Simple watertightness check (for solids/shells)
        if (shapeToMesh.ShapeType() == TopAbs_SOLID || shapeToMesh.ShapeType() == TopAbs_SHELL) {
            // This is a simplified check - a full watertight validation would be more complex
            info.isWatertight = true; // Placeholder - implement proper check if needed
        }

        return true;

    }
    catch (Standard_Failure const& failure) {
        info.failureReason = MESHING_FAILED;
        info.reasonDescription = std::string("Meshing exception: ") + failure.GetMessageString();
        return false;
    }
    catch (...) {
        info.failureReason = MESHING_FAILED;
        info.reasonDescription = "Unknown meshing exception";
        return false;
    }
}

// ========== Decomposition Methods Implementation ==========

void MeshabilitySeparator::DecomposeCompound(const TopoDS_Shape& compound) {
    // Recursively process each component of the compound
    for (TopoDS_Iterator it(compound); it.More(); it.Next()) {
        ProcessShape(it.Value(), false);
    }
}

void MeshabilitySeparator::DecomposeSolid(const TopoDS_Solid& solid) {
    // Process all shells in the solid
    TopTools_MapOfShape processedShells;

    for (TopExp_Explorer shellExp(solid, TopAbs_SHELL); shellExp.More(); shellExp.Next()) {
        const TopoDS_Shape& shell = shellExp.Current();

        // Avoid processing the same shell multiple times
        if (processedShells.Contains(shell)) {
            continue;
        }
        processedShells.Add(shell);

        ProcessShape(shell, false);
    }

    // If solid has no shells, process faces directly
    if (processedShells.Extent() == 0) {
        for (TopExp_Explorer faceExp(solid, TopAbs_FACE); faceExp.More(); faceExp.Next()) {
            ProcessFace(TopoDS::Face(faceExp.Current()));
        }
    }
}

void MeshabilitySeparator::DecomposeShell(const TopoDS_Shell& shell) {
    // Process all faces in the shell
    for (TopExp_Explorer faceExp(shell, TopAbs_FACE); faceExp.More(); faceExp.Next()) {
        ProcessFace(TopoDS::Face(faceExp.Current()));
    }
}

void MeshabilitySeparator::ProcessFace(const TopoDS_Face& face) {
    ShapeAnalysisInfo info = AnalyzeShape(face);

    // Validate face topology and geometry
    if (!CheckFaceValidity(face, info)) {
        info.failureReason = DEGENERATE_FACE;
        BRep_Builder builder;
        builder.Add(nonMeshableParts_, face);
        nonMeshableInfo_.push_back(info);
        UpdateStatistics(info.failureReason, TopAbs_FACE);

        if (useCache_) {
            AddToCache(face, false, info.failureReason);
        }
        return;
    }

    // Attempt to mesh this face
    bool isMeshable = TryMeshing(face, info);

    BRep_Builder builder;
    if (isMeshable) {
        builder.Add(meshableParts_, face);
        meshableInfo_.push_back(info);
    }
    else {
        builder.Add(nonMeshableParts_, face);
        nonMeshableInfo_.push_back(info);
    }

    if (useCache_) {
        AddToCache(face, isMeshable, info.failureReason);
    }

    UpdateStatistics(info.failureReason, TopAbs_FACE);
}

void MeshabilitySeparator::ProcessEdge(const TopoDS_Shape& edge) {
    // Edges cannot be exported to STL directly
    ShapeAnalysisInfo info = AnalyzeShape(edge);
    info.failureReason = UNSUPPORTED_SURFACE;
    info.reasonDescription = "Edge cannot be exported to STL format";

    BRep_Builder builder;
    builder.Add(nonMeshableParts_, edge);
    nonMeshableInfo_.push_back(info);

    UpdateStatistics(info.failureReason, edge.ShapeType());
}

void MeshabilitySeparator::ProcessVertex(const TopoDS_Shape& vertex) {
    // Vertices cannot be exported to STL directly
    ShapeAnalysisInfo info = AnalyzeShape(vertex);
    info.failureReason = UNSUPPORTED_SURFACE;
    info.reasonDescription = "Vertex cannot be exported to STL format";

    BRep_Builder builder;
    builder.Add(nonMeshableParts_, vertex);
    nonMeshableInfo_.push_back(info);

    UpdateStatistics(info.failureReason, vertex.ShapeType());
}

// ========== Shape Analysis and Validation ==========

ShapeAnalysisInfo MeshabilitySeparator::AnalyzeShape(const TopoDS_Shape& shape) {
    ShapeAnalysisInfo info;
    info.shape = shape;

    if (shape.IsNull()) {
        info.failureReason = NULL_GEOMETRY;
        return info;
    }

    TopAbs_ShapeEnum type = shape.ShapeType();

    // Analyze based on shape type
    switch (type) {
    case TopAbs_FACE: {
        TopoDS_Face face = TopoDS::Face(shape);

        // Analyze surface type
        Handle(Geom_Surface) surface = BRep_Tool::Surface(face);
        if (!surface.IsNull()) {
            GeomAdaptor_Surface adaptor(surface);
            GeomAbs_SurfaceType surfType = adaptor.GetType();

            // Flag complex surfaces that might cause meshing issues
            if (surfType == GeomAbs_BezierSurface ||
                surfType == GeomAbs_BSplineSurface ||
                surfType == GeomAbs_SurfaceOfExtrusion ||
                surfType == GeomAbs_SurfaceOfRevolution ||
                surfType == GeomAbs_OffsetSurface ||
                surfType == GeomAbs_OtherSurface) {
                info.hasComplexCurves = true;
            }
        }

        // Analyze edge lengths
        info.minEdgeLength = 1e10; // Large initial value
        info.maxEdgeLength = 0.0;

        for (TopExp_Explorer edgeExp(face, TopAbs_EDGE); edgeExp.More(); edgeExp.Next()) {
            TopoDS_Edge edge = TopoDS::Edge(edgeExp.Current());
            CheckEdgeValidity(edge, info);
        }
        break;
    }

    case TopAbs_EDGE: {
        CheckEdgeValidity(shape, info);
        break;
    }

    default:
        // For other types, basic analysis is sufficient
        break;
    }

    return info;
}

bool MeshabilitySeparator::CheckFaceValidity(const TopoDS_Face& face, ShapeAnalysisInfo& info) {
    if (face.IsNull()) {
        info.failureReason = NULL_GEOMETRY;
        info.reasonDescription = "Face is null";
        return false;
    }

    // Use BRepCheck_Analyzer for comprehensive topological validation
    BRepCheck_Analyzer analyzer(face);
    if (!analyzer.IsValid()) {
        info.failureReason = DEGENERATE_FACE;

        // Collect detailed error information
        std::stringstream ss;
        ss << "Face validation failed. Issues detected: ";
        bool hasEdgeError = false;

        // Check each edge in the face
        for (TopExp_Explorer edgeExp(face, TopAbs_EDGE); edgeExp.More(); edgeExp.Next()) {
            TopoDS_Edge edge = TopoDS::Edge(edgeExp.Current());

            // Get validation result for this edge
            const Handle(BRepCheck_Result)& edgeResult = analyzer.Result(edge);
            if (!edgeResult.IsNull()) {
                // Check if edge has any failures
                const BRepCheck_ListOfStatus& listOfStatus = edgeResult->Status();

                for (BRepCheck_ListIteratorOfListOfStatus statusIt(listOfStatus); statusIt.More(); statusIt.Next()) {
                    BRepCheck_Status status = statusIt.Value();
                    if (status != BRepCheck_NoError) {
                        hasEdgeError = true;
                        ss << "Edge has status: " << StatusToString(status) << "; ";
                    }
                }
            }
        }

        if (hasEdgeError) {
            info.reasonDescription = ss.str();
        }
        else {
            info.reasonDescription = "Face validation failed (non-edge related issue).";
        }
        return false;
    }

    // Additional geometric validation
    Handle(Geom_Surface) surface = BRep_Tool::Surface(face);
    if (surface.IsNull()) {
        info.failureReason = NULL_GEOMETRY;
        info.reasonDescription = "Face has null surface geometry";
        return false;
    }

    return true;
}

bool MeshabilitySeparator::CheckEdgeValidity(const TopoDS_Shape& edge, ShapeAnalysisInfo& info) {
    TopoDS_Edge theEdge = TopoDS::Edge(edge);

    if (theEdge.IsNull()) {
        return false;
    }

    Standard_Real first, last;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(theEdge, first, last);

    if (curve.IsNull()) {
        info.failureReason = NULL_GEOMETRY;
        info.reasonDescription = "Edge has no geometric curve";
        return false;
    }

    try {
        // Calculate edge length
        GeomAdaptor_Curve adaptor(curve, first, last);
        double length = GCPnts_AbscissaPoint::Length(adaptor);

        // Check for degenerate (extremely short) edges
        if (length < Precision::Confusion()) {
            info.failureReason = DEGENERATE_EDGE;
            info.reasonDescription = "Edge length is below precision threshold";
            return false;
        }

        // Update min/max edge length statistics
        if (length < info.minEdgeLength) {
            info.minEdgeLength = length;
        }
        if (length > info.maxEdgeLength) {
            info.maxEdgeLength = length;
        }

        // Check curve type
        GeomAbs_CurveType curveType = adaptor.GetType();
        if (curveType == GeomAbs_BSplineCurve ||
            curveType == GeomAbs_BezierCurve ||
            curveType == GeomAbs_OffsetCurve ||
            curveType == GeomAbs_OtherCurve) {
            info.hasComplexCurves = true;
        }

    }
    catch (...) {
        info.failureReason = DEGENERATE_EDGE;
        info.reasonDescription = "Edge length calculation failed";
        return false;
    }

    return true;
}

// ========== Geometry Repair Methods ==========

TopoDS_Shape MeshabilitySeparator::TryFixShape(const TopoDS_Shape& shape) {
    try {
        Handle(ShapeFix_Shape) fixer = new ShapeFix_Shape(shape);
        fixer->SetPrecision(1e-6);
        fixer->SetMaxTolerance(0.01);
        fixer->Perform();

        if (fixer->Status(ShapeExtend_DONE)) {
            return fixer->Shape();
        }
    }
    catch (...) {
        // Repair failed - return original shape
    }

    return shape;
}

TopoDS_Face MeshabilitySeparator::TryFixFace(const TopoDS_Face& face) {
    try {
        Handle(ShapeFix_Face) fixer = new ShapeFix_Face(face);
        fixer->FixWireTool()->SetPrecision(1e-6);
        fixer->FixWireTool()->SetMaxTolerance(0.01);
        fixer->Perform();

        if (fixer->Status(ShapeExtend_DONE)) {
            return fixer->Face();
        }
    }
    catch (...) {
        // Repair failed - return original face
    }

    return face;
}

// ========== Cache Management Implementation ==========

bool MeshabilitySeparator::IsShapeInCache(const TopoDS_Shape& shape, bool& isMeshable, MeshFailureReason& reason) {
    // Check meshable cache
    Standard_Integer meshableValue = 0;
    if (meshableCache_.Find(shape, meshableValue)) {
        isMeshable = (meshableValue == 1);

        // If not meshable, get failure reason
        if (!isMeshable) {
            Standard_Integer reasonValue = 0;
            if (failureCache_.Find(shape, reasonValue)) {
                reason = static_cast<MeshFailureReason>(reasonValue);
            }
            else {
                reason = OTHER_REASON;
            }
        }
        else {
            reason = SUCCESS;
        }

        return true;
    }

    return false;
}

void MeshabilitySeparator::AddToCache(const TopoDS_Shape& shape, bool isMeshable, MeshFailureReason reason) {
    // Store meshability result
    meshableCache_.Bind(shape, isMeshable ? 1 : 0);

    // Store failure reason if applicable
    if (!isMeshable) {
        failureCache_.Bind(shape, static_cast<Standard_Integer>(reason));
    }
}

void MeshabilitySeparator::ClearCache() {
    meshableCache_.Clear();
    failureCache_.Clear();
}

// ========== Statistics Management ==========

void MeshabilitySeparator::UpdateStatistics(MeshFailureReason reason, TopAbs_ShapeEnum shapeType) {
    stats_.totalShapes++;

    if (reason == SUCCESS) {
        stats_.meshableShapes++;
    }
    else {
        stats_.nonMeshableShapes++;
        stats_.failureCounts[reason]++;
    }

    // Update type-specific counters
    switch (shapeType) {
    case TopAbs_FACE:
        stats_.facesProcessed++;
        break;
    case TopAbs_SOLID:
        stats_.solidsProcessed++;
        break;
    case TopAbs_SHELL:
        stats_.shellsProcessed++;
        break;
    default:
        break;
    }
}

void MeshabilitySeparator::ResetStatistics() {
    stats_.totalShapes = 0;
    stats_.meshableShapes = 0;
    stats_.nonMeshableShapes = 0;
    stats_.facesProcessed = 0;
    stats_.solidsProcessed = 0;
    stats_.shellsProcessed = 0;
    stats_.failureCounts.clear();
}

// ========== Reporting Methods ==========

std::string MeshabilitySeparator::GetAnalysisReport() const {
    return GenerateReport();
}

std::string MeshabilitySeparator::GenerateReport() const {
    std::stringstream report;

    report << "========================================\n";
    report << "     Meshability Analysis Report\n";
    report << "========================================\n\n";

    report << "Processing Parameters:\n";
    report << "  - Deflection: " << deflection_ << "\n";
    report << "  - Angle: " << angle_ << " radians\n";
    report << "  - Geometry Repair: " << (tryFixBeforeMeshing_ ? "Enabled" : "Disabled") << "\n";
    report << "  - Caching: " << (useCache_ ? "Enabled" : "Disabled") << "\n\n";

    report << "Statistics:\n";
    report << "  - Total Shapes: " << stats_.totalShapes << "\n";
    report << "  - Meshable: " << stats_.meshableShapes << "\n";
    report << "  - Non-Meshable: " << stats_.nonMeshableShapes << "\n";

    if (stats_.totalShapes > 0) {
        double successRate = 100.0 * stats_.meshableShapes / stats_.totalShapes;
        report << "  - Success Rate: " << std::fixed << std::setprecision(1) << successRate << "%\n";
    }

    report << "\nProcessing Details:\n";
    report << "  - Faces Processed: " << stats_.facesProcessed << "\n";
    report << "  - Solids Processed: " << stats_.solidsProcessed << "\n";
    report << "  - Shells Processed: " << stats_.shellsProcessed << "\n\n";

    if (!stats_.failureCounts.empty()) {
        report << "Failure Analysis:\n";
        for (const auto& pair : stats_.failureCounts) {
            report << "  - " << FailureReasonToString(pair.first)
                << ": " << pair.second << " occurrences\n";
        }
        report << "\n";
    }

    if (!nonMeshableInfo_.empty()) {
        report << "Non-Meshable Elements (first 10):\n";
        int count = 0;
        for (const auto& info : nonMeshableInfo_) {
            if (count++ >= 10) break;

            report << "  " << count << ". "
                << ShapeTypeToString(info.shape.ShapeType())
                << " - " << FailureReasonToString(info.failureReason);

            if (!info.reasonDescription.empty()) {
                report << " (" << info.reasonDescription << ")";
            }

            if (info.hasComplexCurves) {
                report << " [Contains complex curves]";
            }

            report << "\n";
        }

        if (nonMeshableInfo_.size() > 10) {
            report << "  ... and " << (nonMeshableInfo_.size() - 10)
                << " more elements\n";
        }
        report << "\n";
    }

    report << "========================================\n";

    return report.str();
}

std::string MeshabilitySeparator::FailureReasonToString(MeshFailureReason reason) const {
    switch (reason) {
    case SUCCESS: return "Success";
    case NULL_GEOMETRY: return "Null Geometry";
    case MESHING_FAILED: return "Meshing Failed";
    case NO_TRIANGLES: return "No Triangles Generated";
    case DEGENERATE_EDGE: return "Degenerate Edge";
    case DEGENERATE_FACE: return "Degenerate Face";
    case SELF_INTERSECTING: return "Self-Intersecting";
    case NON_MANIFOLD: return "Non-Manifold";
    case COMPLEX_CURVE: return "Complex Curve";
    case UNSUPPORTED_SURFACE: return "Unsupported Surface";
    case OTHER_REASON: return "Other Reason";
    default: return "Unknown";
    }
}

std::string MeshabilitySeparator::ShapeTypeToString(TopAbs_ShapeEnum type) const {
    switch (type) {
    case TopAbs_COMPOUND: return "Compound";
    case TopAbs_COMPSOLID: return "CompSolid";
    case TopAbs_SOLID: return "Solid";
    case TopAbs_SHELL: return "Shell";
    case TopAbs_FACE: return "Face";
    case TopAbs_WIRE: return "Wire";
    case TopAbs_EDGE: return "Edge";
    case TopAbs_VERTEX: return "Vertex";
    case TopAbs_SHAPE: return "Shape";
    default: return "Unknown Type";
    }
}