#ifndef MESHABILITY_SEPARATOR_HXX
#define MESHABILITY_SEPARATOR_HXX

#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <Standard_Real.hxx>
#include <TopTools_DataMapOfShapeInteger.hxx>
#include <string>
#include <vector>
#include <map>

// Forward declarations for OCCT handles
class TopoDS_Face;
class TopoDS_Solid;
class TopoDS_Shell;
class Poly_Triangulation;

/**
 * @enum MeshFailureReason
 * @brief Enumerates possible reasons for meshing failure.
 * Used to categorize why a specific shape could not be converted to a mesh.
 */
enum MeshFailureReason {
    SUCCESS = 0,            ///< Meshing successful
    NULL_GEOMETRY,          ///< Underlying geometry is null
    MESHING_FAILED,         ///< Meshing algorithm failed
    NO_TRIANGLES,           ///< No triangles generated
    DEGENERATE_EDGE,        ///< Edge is degenerate (e.g., zero length)
    DEGENERATE_FACE,        ///< Face is degenerate
    SELF_INTERSECTING,      ///< Shape contains self-intersections
    NON_MANIFOLD,           ///< Shape is non-manifold
    COMPLEX_CURVE,          ///< Contains complex curves (e.g., B-spline)
    UNSUPPORTED_SURFACE,    ///< Contains unsupported surface types
    OTHER_REASON            ///< Other unspecified reasons
};

/**
 * @struct ShapeAnalysisInfo
 * @brief Stores detailed analysis results for a single shape.
 * Contains both topological/geometric properties and meshing outcome.
 */
struct ShapeAnalysisInfo {
    TopoDS_Shape shape;                 ///< The shape being analyzed
    MeshFailureReason failureReason;    ///< Reason for meshing failure (if any)
    std::string reasonDescription;      ///< Human-readable failure description
    double minEdgeLength;               ///< Minimum edge length in the shape
    double maxEdgeLength;               ///< Maximum edge length in the shape
    bool hasComplexCurves;              ///< True if shape contains complex curves
    bool isWatertight;                  ///< True if shape forms a closed volume
    int faceCount;                      ///< Number of faces in the shape
    int triangleCount;                  ///< Number of triangles generated

    /// Constructor with default initialization
    ShapeAnalysisInfo() :
        failureReason(SUCCESS),
        minEdgeLength(0.0),
        maxEdgeLength(0.0),
        hasComplexCurves(false),
        isWatertight(false),
        faceCount(0),
        triangleCount(0) {
    }
};

/**
 * @class MeshabilitySeparator
 * @brief Main class implementing the recursive decomposition and test-meshing strategy.
 *
 * This class separates a CAD model into meshable and non-meshable parts by recursively
 * decomposing the model and attempting to mesh each component. It handles various
 * topological types (Compounds, Solids, Shells, Faces) and provides detailed analysis.
 */
class MeshabilitySeparator {
public:
    // ========== Constructor and Configuration ==========

    /**
     * @brief Constructor with configurable meshing parameters.
     * @param deflection Maximum chordal deviation for meshing (defines accuracy)
     * @param angle Angular deflection for meshing
     * @param relative If true, deflection is relative to shape size
     * @param parallel Enable parallel processing for meshing
     */
    explicit MeshabilitySeparator(
        double deflection = 0.01,
        double angle = 0.5,
        bool relative = false,
        bool parallel = true
    );

    // ========== Main Public Interface ==========

    /**
     * @brief Main method to separate a shape into meshable and non-meshable parts.
     * @param inputShape The input CAD model to analyze
     * @param meshableParts Output compound containing meshable components
     * @param nonMeshableParts Output compound containing non-meshable components
     * @return True if separation completed successfully (even if some parts failed)
     */
    bool Separate(
        const TopoDS_Shape& inputShape,
        TopoDS_Compound& meshableParts,
        TopoDS_Compound& nonMeshableParts
    );

    /// Converts shape type enum to human-readable string
    std::string ShapeTypeToString(TopAbs_ShapeEnum type) const;

    // ========== Analysis and Reporting ==========

    /**
     * @brief Generates a comprehensive text report of the analysis results.
     * @return Formatted string containing statistics and failure analysis.
     */
    std::string GetAnalysisReport() const;

    /// Returns detailed information about meshable shapes
    const std::vector<ShapeAnalysisInfo>& GetMeshableInfo() const { return meshableInfo_; }

    /// Returns detailed information about non-meshable shapes
    const std::vector<ShapeAnalysisInfo>& GetNonMeshableInfo() const { return nonMeshableInfo_; }

    /**
     * @struct Statistics
     * @brief Collection of statistical counters about the separation process.
     */
    struct Statistics {
        int totalShapes;                            ///< Total number of shapes processed
        int meshableShapes;                         ///< Number of successfully meshed shapes
        int nonMeshableShapes;                      ///< Number of shapes that failed meshing
        int facesProcessed;                         ///< Number of faces processed
        int solidsProcessed;                        ///< Number of solids processed
        int shellsProcessed;                        ///< Number of shells processed
        std::map<MeshFailureReason, int> failureCounts; ///< Count of failures by reason
    };

    /// Returns the current statistics
    const Statistics& GetStatistics() const { return stats_; }

    // ========== Configuration Setters/Getters ==========

    void SetDeflection(double deflection) { deflection_ = deflection; }
    double GetDeflection() const { return deflection_; }

    void SetAngle(double angle) { angle_ = angle; }
    double GetAngle() const { return angle_; }

    void SetTryFixBeforeMeshing(bool tryFix) { tryFixBeforeMeshing_ = tryFix; }
    bool GetTryFixBeforeMeshing() const { return tryFixBeforeMeshing_; }

    void EnableCaching(bool enable) { useCache_ = enable; }
    bool IsCachingEnabled() const { return useCache_; }

    /// Clears all internal caches
    void ClearCache();

private:
    // ========== Configuration Parameters ==========
    double deflection_;          ///< Chordal deflection for meshing
    double angle_;               ///< Angular deflection for meshing
    bool relative_;              ///< Relative deflection flag
    bool parallel_;              ///< Parallel processing flag
    bool tryFixBeforeMeshing_;   ///< Attempt geometry repair before meshing
    bool useCache_;              ///< Enable result caching for performance

    // ========== Results Storage ==========
    TopoDS_Compound meshableParts_;      ///< Compound of meshable shapes
    TopoDS_Compound nonMeshableParts_;   ///< Compound of non-meshable shapes
    std::vector<ShapeAnalysisInfo> meshableInfo_;     ///< Info for meshable shapes
    std::vector<ShapeAnalysisInfo> nonMeshableInfo_;  ///< Info for non-meshable shapes
    Statistics stats_;                   ///< Processing statistics

    // ========== Cache Implementation ==========
    // Using OCCT's native map containers for reliable shape caching
    TopTools_DataMapOfShapeInteger meshableCache_;     ///< Shape -> 1=meshable, 0=non-meshable
    TopTools_DataMapOfShapeInteger failureCache_;      ///< Shape -> failure reason code

    // ========== Core Processing Methods ==========

    /**
     * @brief Recursively processes a shape based on its topological type.
     * @param shape The shape to process
     * @param topLevel True if this is the top-level call (for statistics)
     * @return True if the shape itself is meshable (not considering children)
     */
    bool ProcessShape(const TopoDS_Shape& shape, bool topLevel = true);

    /**
     * @brief Attempts to mesh a shape and records the outcome.
     * @param shape Shape to attempt meshing on
     * @param info Analysis info structure to populate with results
     * @return True if meshing succeeded
     */
    bool TryMeshing(const TopoDS_Shape& shape, ShapeAnalysisInfo& info);

    // ========== Decomposition Methods ==========

    /// Decomposes a COMPOUND or COMPSOLID into its components
    void DecomposeCompound(const TopoDS_Shape& compound);

    /// Decomposes a SOLID into its constituent SHELLs
    void DecomposeSolid(const TopoDS_Solid& solid);

    /// Decomposes a SHELL into its constituent FACEs
    void DecomposeShell(const TopoDS_Shell& shell);

    /// Processes a single FACE (lowest decomposition level)
    void ProcessFace(const TopoDS_Face& face);

    /// Processes edges (cannot be meshed alone)
    void ProcessEdge(const TopoDS_Shape& edge);

    /// Processes vertices (cannot be meshed alone)
    void ProcessVertex(const TopoDS_Shape& vertex);

    // ========== Analysis and Validation Methods ==========

    /**
     * @brief Analyzes geometric properties of a shape.
     * @param shape Shape to analyze
     * @return AnalysisInfo structure with geometric properties
     */
    ShapeAnalysisInfo AnalyzeShape(const TopoDS_Shape& shape);

    /**
     * @brief Validates topological and geometric correctness of a face.
     * @param face Face to validate
     * @param info Analysis info to update with validation results
     * @return True if face is valid
     */
    bool CheckFaceValidity(const TopoDS_Face& face, ShapeAnalysisInfo& info);

    /**
     * @brief Validates an edge and computes its geometric properties.
     * @param edge Edge to validate
     * @param info Analysis info to update
     * @return True if edge is valid
     */
    bool CheckEdgeValidity(const TopoDS_Shape& edge, ShapeAnalysisInfo& info);

    // ========== Geometry Repair Methods ==========

    /// Attempts to repair topological issues in a shape
    TopoDS_Shape TryFixShape(const TopoDS_Shape& shape);

    /// Attempts to repair issues in a specific face
    TopoDS_Face TryFixFace(const TopoDS_Face& face);

    // ========== Cache Management Methods ==========

    /**
     * @brief Checks if a shape is in the cache and retrieves its status.
     * @param shape Shape to look up
     * @param isMeshable Output: true if shape is meshable
     * @param reason Output: failure reason if not meshable
     * @return True if shape was found in cache
     */
    bool IsShapeInCache(const TopoDS_Shape& shape, bool& isMeshable, MeshFailureReason& reason);

    /**
     * @brief Adds a shape's meshing result to the cache.
     * @param shape Shape to cache
     * @param isMeshable True if shape is meshable
     * @param reason Failure reason if not meshable
     */
    void AddToCache(const TopoDS_Shape& shape, bool isMeshable, MeshFailureReason reason = SUCCESS);

    // ========== Statistics and Reporting ==========

    /// Updates statistics counters
    void UpdateStatistics(MeshFailureReason reason, TopAbs_ShapeEnum shapeType);

    /// Resets all statistics to zero
    void ResetStatistics();

    /// Generates the detailed analysis report
    std::string GenerateReport() const;

    /// Converts failure reason enum to human-readable string
    std::string FailureReasonToString(MeshFailureReason reason) const;
};

#endif // MESHABILITY_SEPARATOR_HXX