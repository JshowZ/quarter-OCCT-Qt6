#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <fstream>
#include <chrono>
#include <TopoDS_Shape.hxx>
#include <STEPControl_Reader.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepTools.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <TColStd_HSequenceOfTransient.hxx>
#include <STEPConstruct.hxx>
#include <Interface_Static.hxx>

// Entity status definition
enum class EntityStatus {
    EXPORTABLE,     // Can be exported
    PROBLEMATIC,    // Has issues but fixable
    UNSUPPORTED,    // Unsupported entity type
    SKIPPED         // Skipped
};

// Entity information structure
struct EntityInfo {
    int id;                         // Entity ID
    std::string name;               // Entity name
    std::string type;               // STEP type
    TopAbs_ShapeEnum shapeType;     // OpenCASCADE shape type
    EntityStatus status;            // Status
    std::vector<std::string> issues; // Issue list
    TopoDS_Shape shape;             // Shape object
    double volume;                  // Volume (if applicable)
    double surfaceArea;             // Surface area
    bool isManifold;                // Whether it's manifold
    bool isClosed;                  // Whether it's closed
    int faceCount;                  // Face count
    int edgeCount;                  // Edge count
    int vertexCount;                // Vertex count
};

// Analysis result structure
struct AnalysisResult {
    std::vector<EntityInfo> exportableEntities;
    std::vector<EntityInfo> problematicEntities;
    std::vector<EntityInfo> unsupportedEntities;
    std::vector<EntityInfo> skippedEntities;

    int totalEntities = 0;
    int successfulMeshCount = 0;
    double processingTime = 0.0;

    // Statistics
    std::map<std::string, int> entityTypeCount;
    std::map<std::string, std::vector<std::string>> issueCategories;
};

class STEPAnalyzer {
public:
    STEPAnalyzer();
    ~STEPAnalyzer();

    // Load STEP file
    bool LoadSTEP(const std::string& filepath);

    // Analyze all entities
    AnalysisResult Analyze(bool quickMode = false);

    // Analyze single entity
    EntityInfo AnalyzeEntity(const TopoDS_Shape& shape, int entityId);

    // Try to fix problematic entity
    bool TryFixEntity(EntityInfo& entity);

    // Export exportable entities to STL
    bool ExportSTL(const std::string& outputPath);

    // Generate detailed report
    std::string GenerateReport() const;

    // Set mesh parameters
    void SetMeshParams(double linearDeflection = 0.01,
        double angularDeflection = 0.5,
        bool relativeMode = true);

    // Set whether to include surfaces
    void SetIncludeSurfaces(bool include) { m_includeSurfaces = include; }

    // Set whether to auto fix issues
    void SetAutoFix(bool autoFix) { m_autoFix = autoFix; }

private:
    // Internal methods
    bool CheckExportability(const TopoDS_Shape& shape, EntityInfo& info);
    bool PerformMeshTest(const TopoDS_Shape& shape, EntityInfo& info);
    bool CheckManifold(const TopoDS_Shape& shape);
    bool CheckClosed(const TopoDS_Shape& shape);
    double CalculateVolume(const TopoDS_Shape& shape);
    double CalculateSurfaceArea(const TopoDS_Shape& shape);
    void CountElements(const TopoDS_Shape& shape, EntityInfo& info);
    std::string GetSTEPEntityName(int entityId);
    TopAbs_ShapeEnum GetShapeType(const TopoDS_Shape& shape);
    bool IsSupportedType(TopAbs_ShapeEnum shapeType);
    bool HasValidGeometry(const TopoDS_Shape& shape);
    bool CheckSelfIntersection(const TopoDS_Shape& shape);
    bool CheckSmallEdges(const TopoDS_Shape& shape, double tolerance = 1e-6);

    // Fix methods
    TopoDS_Shape FixShape(const TopoDS_Shape& shape);
    TopoDS_Shape RemoveSmallEdges(const TopoDS_Shape& shape, double tolerance);
    TopoDS_Shape SewFaces(const TopoDS_Shape& shape, double tolerance);

    // Member variables
    STEPControl_Reader m_reader;
    TopoDS_Shape m_rootShape;
    Handle(TColStd_HSequenceOfTransient) m_entitySequence;
    AnalysisResult m_result;

    // Configuration
    double m_linearDeflection;
    double m_angularDeflection;
    bool m_relativeMode;
    bool m_includeSurfaces;
    bool m_autoFix;
    bool m_loaded;
};