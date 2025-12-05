#pragma once

#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <Standard_Real.hxx>

class STLExportFilter {
public:
    /**
     * @brief Constructor with default mesh parameters
     * @param deflection Linear deflection for meshing (default: 0.01)
     */
    explicit STLExportFilter(double deflection = 0.01);
    ~STLExportFilter();

    /**
     * @brief Separate model into STL-exportable and non-exportable parts
     * @param inputShape Input model shape
     * @param exportableParts Output compound containing exportable shapes
     * @param nonExportableParts Output compound containing non-exportable shapes
     * @return True if separation completed successfully
     */
    bool separate(const TopoDS_Shape& inputShape, 
                 TopoDS_Compound& exportableParts, 
                 TopoDS_Compound& nonExportableParts);

    /**
     * @brief Set mesh deflection parameter
     * @param deflection Linear deflection for meshing
     */
    void setDeflection(double deflection);

private:
    /**
     * @brief Check if a shape can be exported to STL
     * @param shape Shape to check
     * @return True if shape can be exported to STL
     */
    bool isExportableToSTL(const TopoDS_Shape& shape);

    /**
     * @brief Create mesh for the shape
     * @param shape Shape to mesh
     * @return True if meshing succeeded
     */
    bool createMesh(const TopoDS_Shape& shape);

    /**
     * @brief Check if shape has valid triangulation
     * @param shape Shape to check
     * @return True if shape has valid triangulation
     */
    bool hasValidTriangulation(const TopoDS_Shape& shape);

    /**
     * @brief Recursively process shape and its sub-shapes
     * @param shape Shape to process
     * @param exportableParts Compound for exportable shapes
     * @param nonExportableParts Compound for non-exportable shapes
     */
    void processShapeRecursive(const TopoDS_Shape& shape, 
                              TopoDS_Compound& exportableParts, 
                              TopoDS_Compound& nonExportableParts);

    double deflection_; ///< Linear deflection for meshing
};
