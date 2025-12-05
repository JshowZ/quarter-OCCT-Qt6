#include "STLExportFilter.h"

// OCCT Includes
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <BRep_Builder.hxx>
#include <StlAPI_Writer.hxx>
#include <Precision.hxx>
#include <TopAbs.hxx>

// Standard Includes
#include <iostream>

STLExportFilter::STLExportFilter(double deflection)
    : deflection_(deflection)
{
}

STLExportFilter::~STLExportFilter()
{
}

bool STLExportFilter::separate(const TopoDS_Shape& inputShape, 
                              TopoDS_Compound& exportableParts, 
                              TopoDS_Compound& nonExportableParts)
{
    // Initialize compounds
    BRep_Builder builder;
    builder.MakeCompound(exportableParts);
    builder.MakeCompound(nonExportableParts);

    // Process shape recursively
    processShapeRecursive(inputShape, exportableParts, nonExportableParts);

    return true;
}

void STLExportFilter::setDeflection(double deflection)
{
    deflection_ = deflection;
}

bool STLExportFilter::isExportableToSTL(const TopoDS_Shape& shape)
{
    // Skip empty shapes
    if (shape.IsNull()) {
        return false;
    }

    // Skip vertices and edges (can't be exported to STL)
    TopAbs_ShapeEnum shapeType = shape.ShapeType();
    if (shapeType == TopAbs_VERTEX || shapeType == TopAbs_EDGE) {
        return false;
    }

    // Try to mesh the shape
    if (!createMesh(shape)) {
        return false;
    }

    // Check if mesh has valid triangulation
    return hasValidTriangulation(shape);
}

bool STLExportFilter::createMesh(const TopoDS_Shape& shape)
{
    try {
        // Create mesh using BRepMesh_IncrementalMesh
        BRepMesh_IncrementalMesh meshBuilder(shape, deflection_);
        meshBuilder.Perform();

        return meshBuilder.IsDone();
    } catch (...) {
        return false;
    }
}

bool STLExportFilter::hasValidTriangulation(const TopoDS_Shape& shape)
{
    try {
        // Check if shape has valid triangulation
        bool hasTriangles = false;
        
        // Explore all faces in the shape
        TopExp_Explorer faceExplorer(shape, TopAbs_FACE);
        while (faceExplorer.More()) {
            const TopoDS_Face& face = TopoDS::Face(faceExplorer.Current());
            
            // Get triangulation for the face
            TopLoc_Location loc;
            Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, loc);
            
            if (!triangulation.IsNull() && triangulation->NbTriangles() > 0) {
                hasTriangles = true;
                break;
            }
            
            faceExplorer.Next();
        }
        
        return hasTriangles;
    } catch (...) {
        return false;
    }
}

void STLExportFilter::processShapeRecursive(const TopoDS_Shape& shape, 
                                           TopoDS_Compound& exportableParts, 
                                           TopoDS_Compound& nonExportableParts)
{
    // Check if current shape can be exported to STL
    if (isExportableToSTL(shape)) {
        // Add to exportable parts if it can be exported
        BRep_Builder builder;
        builder.Add(exportableParts, shape);
        return;
    }
    
    // If shape can't be exported as a whole, try to decompose it
    TopAbs_ShapeEnum shapeType = shape.ShapeType();
    bool decomposed = false;
    
    switch (shapeType) {
    case TopAbs_COMPOUND:
    case TopAbs_COMPSOLID:
    case TopAbs_SOLID:
    case TopAbs_SHELL:
        // These shape types can be decomposed further
        decomposed = true;
        break;
    default:
        // These shape types can't be decomposed further
        decomposed = false;
        break;
    }
    
    if (decomposed) {
        // Decompose the shape and process sub-shapes
        TopAbs_ShapeEnum subShapeType;
        
        // Determine appropriate sub-shape type based on parent shape type
        switch (shapeType) {
        case TopAbs_COMPOUND:
            subShapeType = TopAbs_SOLID;
            break;
        case TopAbs_COMPSOLID:
            subShapeType = TopAbs_SOLID;
            break;
        case TopAbs_SOLID:
            subShapeType = TopAbs_SHELL;
            break;
        case TopAbs_SHELL:
            subShapeType = TopAbs_FACE;
            break;
        default:
            subShapeType = TopAbs_FACE;
            break;
        }
        
        TopExp_Explorer subShapeExplorer(shape, subShapeType);
        while (subShapeExplorer.More()) {
            const TopoDS_Shape& subShape = subShapeExplorer.Current();
            processShapeRecursive(subShape, exportableParts, nonExportableParts);
            subShapeExplorer.Next();
        }
    } else {
        // Can't decompose further, add to non-exportable parts
        BRep_Builder builder;
        builder.Add(nonExportableParts, shape);
    }
}
