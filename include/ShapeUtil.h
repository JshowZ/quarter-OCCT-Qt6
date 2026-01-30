#ifndef SHAPEUTIL_H
#define SHAPEUTIL_H

// OCCT includes
#include <TopoDS_Shape.hxx>

// Quarter includes
#include <Inventor/nodes/SoNode.h>

// Shape utility class for OCCT shape conversion
class ShapeUtil
{
public:
    // Convert OCCT shape to Inventor node
    static SoNode* convertShapeRecursive(TopoDS_Shape shape, double deviation = 0.01, double angularDeflection = 0.5);
};

#endif // SHAPEUTIL_H