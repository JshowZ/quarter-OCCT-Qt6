#include "ShapeUtil.h"
#include <QMessageBox>

// OCCT includes
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Polygon3D.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <IMeshTools_Parameters.hxx>
#include <Precision.hxx>
#include <gp_Trsf.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <TopAbs.hxx>
#include <TColgp_Array1OfDir.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

// Quarter includes
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoNormal.h>
#include <Inventor/nodes/SoNormalBinding.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoIndexedLineSet.h>
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/SbVec3f.h>


// Project includes
#include "ViewTool.h"
#include "base.h"

Bnd_Box getBounds(const TopoDS_Shape& shape)
{
    Bnd_Box bounds;
    BRepBndLib::Add(shape, bounds);
    bounds.SetGap(0.0);

    return bounds;
}

Standard_Real getDeflection(const Bnd_Box& bounds, double deviation)
{
    Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
    bounds.Get(xMin, yMin, zMin, xMax, yMax, zMax);

    // calculating the deflection value
    return ((xMax - xMin) + (yMax - yMin) + (zMax - zMin)) / 300.0 * deviation;
}

Standard_Real getDeflection(const TopoDS_Shape& shape, double deviation)
{
    return getDeflection(getBounds(shape), deviation);
}

// ShapeUtil implementation
SoNode* ShapeUtil::convertShapeRecursive(TopoDS_Shape shape, double deviation, double angularDeflection)
{
    try
    {
        SoSeparator* shapeSep = new SoSeparator;
        SoCoordinate3* coords = new SoCoordinate3;
        coords->ref();
        SoNormal* norm = new SoNormal;
        norm->ref();
        SoNormalBinding* normalBinding = new SoNormalBinding;
        normalBinding->ref();
        SoIndexedFaceSet* faceSet = new SoIndexedFaceSet;
        faceSet->ref();
        SoIndexedLineSet* lineSet = new SoIndexedLineSet;
        lineSet->ref();
        SoPointSet* pointSet = new SoPointSet;
        pointSet->ref();

        if (ViewTool::isShapeEmpty(shape)) {
            coords->point.setNum(0);
            norm->vector.setNum(0);
            faceSet->coordIndex.setNum(0);
            lineSet->coordIndex.setNum(0);
            pointSet->startIndex.setValue(0);
            return shapeSep;
        }

        int numTriangles = 0, numNodes = 0, numNorms = 0, numFaces = 0, numEdges = 0, numLines = 0;

        std::set<int> faceEdges;

        Standard_Real deflection = getDeflection(shape, deviation);

        if (deflection < gp::Resolution()) {
            deflection = Precision::Confusion();
        }


        Standard_Real AngDeflectionRads = angularDeflection;

        IMeshTools_Parameters meshParams;
        meshParams.Deflection = deflection;
        meshParams.Relative = Standard_False;
        meshParams.Angle = AngDeflectionRads;
        meshParams.InParallel = Standard_True;
        meshParams.AllowQualityDecrease = Standard_True;

        BRepMesh_IncrementalMesh importMesh(shape, meshParams);


        TopLoc_Location aLoc;
        shape.Location(aLoc);

        // count triangles and nodes in the mesh
        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(shape, TopAbs_FACE, faceMap);
        for (int i = 1; i <= faceMap.Extent(); i++) {
            Handle(Poly_Triangulation) mesh = BRep_Tool::Triangulation(TopoDS::Face(faceMap(i)), aLoc);

            if (mesh.IsNull()) {
                mesh = ViewTool::triangulationOfFace(TopoDS::Face(faceMap(i)));
            }

            // Note: we must also count empty faces
            if (!mesh.IsNull()) {
                numTriangles += mesh->NbTriangles();
                numNodes += mesh->NbNodes();
                numNorms += mesh->NbNodes();
            }

            TopExp_Explorer xp;
            for (xp.Init(faceMap(i), TopAbs_EDGE); xp.More(); xp.Next()) {
                faceEdges.insert(Jumpers::ShapeMapHasher{}(xp.Current()));
            }
            numFaces++;
        }

        // get an indexed map of edges
        TopTools_IndexedMapOfShape edgeMap;
        TopExp::MapShapes(shape, TopAbs_EDGE, edgeMap);

        // key is the edge number, value the coord indexes. This is needed to keep the same order as
        // the edges.
        std::map<int, std::vector<int32_t>> lineSetMap;
        std::set<int> edgeIdxSet;
        std::vector<int32_t> edgeVector;

        // count and index the edges
        for (int i = 1; i <= edgeMap.Extent(); i++) {
            edgeIdxSet.insert(i);
            numEdges++;

            const TopoDS_Edge& aEdge = TopoDS::Edge(edgeMap(i));
            TopLoc_Location aLoc;


            int hash = Jumpers::ShapeMapHasher{}(aEdge);
            if (faceEdges.find(hash) == faceEdges.end()) {
                Handle(Poly_Polygon3D) aPoly = ViewTool::polygonOfEdge(aEdge, aLoc);
                if (!aPoly.IsNull()) {
                    int nbNodesInEdge = aPoly->NbNodes();
                    numNodes += nbNodesInEdge;
                }
            }
        }

        // handling of the vertices
        TopTools_IndexedMapOfShape vertexMap;
        TopExp::MapShapes(shape, TopAbs_VERTEX, vertexMap);
        numNodes += vertexMap.Extent();

        // create memory for the nodes and indexes
        coords->point.setNum(numNodes);
        norm->vector.setNum(numNorms);
        faceSet->coordIndex.setNum(numTriangles * 4);
        //faceSet->partIndex.setNum(numFaces);

        // get the raw memory for fast fill up
        SbVec3f* verts = coords->point.startEditing();
        SbVec3f* norms = norm->vector.startEditing();
        int32_t* index = faceSet->coordIndex.startEditing();
        // int32_t* parts = faceSet->partIndex.startEditing();

         // preset the normal vector with null vector
        for (int i = 0; i < numNorms; i++) {
            norms[i] = SbVec3f(0.0, 0.0, 0.0);
        }

        int ii = 0, faceNodeOffset = 0, faceTriaOffset = 0;
        for (int i = 1; i <= faceMap.Extent(); i++, ii++) {
            TopLoc_Location aLoc;
            const TopoDS_Face& actFace = TopoDS::Face(faceMap(i));
            // get the mesh of the shape
            Handle(Poly_Triangulation) mesh = BRep_Tool::Triangulation(actFace, aLoc);
            if (mesh.IsNull()) {
                mesh = ViewTool::triangulationOfFace(actFace);
                continue;
            }

            //if (mesh.IsNull()) {
            //    parts[ii] = 0;
            //    continue;
            //}

            // getting the transformation of the shape/face
            gp_Trsf myTransf;
            Standard_Boolean identity = true;
            if (!aLoc.IsIdentity()) {
                identity = false;
                myTransf = aLoc.Transformation();
            }

            // getting size of node and triangle array of this face
            int nbNodesInFace = mesh->NbNodes();
            int nbTriInFace = mesh->NbTriangles();
            // check orientation
            TopAbs_Orientation orient = actFace.Orientation();

            int numNodes = mesh->NbNodes();
            TColgp_Array1OfDir Normals(1, numNodes);

            //if (normalsFromUV) {
            //    Part::Tools::getPointNormals(actFace, mesh, Normals);
            //}

            for (int g = 1; g <= nbTriInFace; g++) {
                // Get the triangle
                Standard_Integer N1, N2, N3;
                mesh->Triangle(g).Get(N1, N2, N3);

                // change orientation of the triangle if the face is reversed
                if (orient != TopAbs_FORWARD) {
                    Standard_Integer tmp = N1;
                    N1 = N2;
                    N2 = tmp;
                }


                gp_Pnt V1(mesh->Node(N1)), V2(mesh->Node(N2)), V3(mesh->Node(N3));

                // get the 3 normals of this triangle
                gp_Vec NV1, NV2, NV3;
                //if (normalsFromUV) {
                //    NV1.SetXYZ(Normals(N1).XYZ());
                //    NV2.SetXYZ(Normals(N2).XYZ());
                //    NV3.SetXYZ(Normals(N3).XYZ());
                //}
                //else {
                //    gp_Vec v1 = Base::convertTo<gp_Vec>(V1);
                //    gp_Vec v2 = Base::convertTo<gp_Vec>(V2);
                //    gp_Vec v3 = Base::convertTo<gp_Vec>(V3);

                //    gp_Vec normal = (v2 - v1) ^ (v3 - v1);
                //    NV1 = normal;
                //    NV2 = normal;
                //    NV3 = normal;
                //}

                gp_Vec v1 = Base::convertTo<gp_Vec>(V1);
                gp_Vec v2 = Base::convertTo<gp_Vec>(V2);
                gp_Vec v3 = Base::convertTo<gp_Vec>(V3);

                gp_Vec normal = (v2 - v1) ^ (v3 - v1);
                NV1 = normal;
                NV2 = normal;
                NV3 = normal;

                // transform the vertices and normals to the place of the face
                if (!identity) {
                    V1.Transform(myTransf);
                    V2.Transform(myTransf);
                    V3.Transform(myTransf);
                }

                // add the normals for all points of this triangle
                norms[faceNodeOffset + N1 - 1] += Base::convertTo<SbVec3f>(NV1);
                norms[faceNodeOffset + N2 - 1] += Base::convertTo<SbVec3f>(NV2);
                norms[faceNodeOffset + N3 - 1] += Base::convertTo<SbVec3f>(NV3);

                // set the vertices
                verts[faceNodeOffset + N1 - 1] = Base::convertTo<SbVec3f>(V1);
                verts[faceNodeOffset + N2 - 1] = Base::convertTo<SbVec3f>(V2);
                verts[faceNodeOffset + N3 - 1] = Base::convertTo<SbVec3f>(V3);

                // set the index vector with the 3 point indexes and the end delimiter
                index[faceTriaOffset * 4 + 4 * (g - 1)] = faceNodeOffset + N1 - 1;
                index[faceTriaOffset * 4 + 4 * (g - 1) + 1] = faceNodeOffset + N2 - 1;
                index[faceTriaOffset * 4 + 4 * (g - 1) + 2] = faceNodeOffset + N3 - 1;
                index[faceTriaOffset * 4 + 4 * (g - 1) + 3] = SO_END_FACE_INDEX;
            }

            //parts[ii] = nbTriInFace;  // new part

            // handling the edges lying on this face
            TopExp_Explorer Exp;
            for (Exp.Init(actFace, TopAbs_EDGE); Exp.More(); Exp.Next()) {
                const TopoDS_Edge& curEdge = TopoDS::Edge(Exp.Current());
                // get the overall index of this edge
                int edgeIndex = edgeMap.FindIndex(curEdge);
                edgeVector.push_back((int32_t)edgeIndex - 1);
                // already processed this index ?
                if (edgeIdxSet.find(edgeIndex) != edgeIdxSet.end()) {

                    // this holds the indices of the edge's triangulation to the current polygon
                    Handle(Poly_PolygonOnTriangulation) aPoly =
                        BRep_Tool::PolygonOnTriangulation(curEdge, mesh, aLoc);
                    if (aPoly.IsNull()) {
                        continue;  // polygon does not exist
                    }

                    // getting the indexes of the edge polygon
                    const TColStd_Array1OfInteger& indices = aPoly->Nodes();
                    for (Standard_Integer i = indices.Lower(); i <= indices.Upper(); i++) {
                        int nodeIndex = indices(i);
                        int index = faceNodeOffset + nodeIndex - 1;
                        lineSetMap[edgeIndex].push_back(index);

                        // usually the coordinates for this edge are already set by the
                        // triangles of the face this edge belongs to. However, there are
                        // rare cases where some points are only referenced by the polygon
                        // but not by any triangle. Thus, we must apply the coordinates to
                        // make sure that everything is properly set.

                        gp_Pnt p(mesh->Node(nodeIndex));
                        if (!identity) {
                            p.Transform(myTransf);
                        }
                        verts[index] = Base::convertTo<SbVec3f>(p);
                    }

                    // remove the handled edge index from the set
                    edgeIdxSet.erase(edgeIndex);
                }
            }

            edgeVector.push_back(-1);

            // counting up the per Face offsets
            faceNodeOffset += nbNodesInFace;
            faceTriaOffset += nbTriInFace;
        }

        // handling of the free edges
        for (int i = 1; i <= edgeMap.Extent(); i++) {
            const TopoDS_Edge& aEdge = TopoDS::Edge(edgeMap(i));
            Standard_Boolean identity = true;
            gp_Trsf myTransf;
            TopLoc_Location aLoc;

            // handling of the free edge that are not associated to a face
            int hash = Jumpers::ShapeMapHasher{}(aEdge);
            if (faceEdges.find(hash) == faceEdges.end()) {
                Handle(Poly_Polygon3D) aPoly = ViewTool::polygonOfEdge(aEdge, aLoc);
                if (!aPoly.IsNull()) {
                    if (!aLoc.IsIdentity()) {
                        identity = false;
                        myTransf = aLoc.Transformation();
                    }

                    const TColgp_Array1OfPnt& aNodes = aPoly->Nodes();
                    int nbNodesInEdge = aPoly->NbNodes();

                    gp_Pnt pnt;
                    for (Standard_Integer j = 1; j <= nbNodesInEdge; j++) {
                        pnt = aNodes(j);
                        if (!identity) {
                            pnt.Transform(myTransf);
                        }
                        int index = faceNodeOffset + j - 1;
                        verts[index] = Base::convertTo<SbVec3f>(pnt);
                        lineSetMap[i].push_back(index);
                    }

                    faceNodeOffset += nbNodesInEdge;
                }
            }
        }

        pointSet->startIndex.setValue(faceNodeOffset);
        for (int i = 0; i < vertexMap.Extent(); i++) {
            const TopoDS_Vertex& aVertex = TopoDS::Vertex(vertexMap(i + 1));
            gp_Pnt pnt = BRep_Tool::Pnt(aVertex);

            verts[faceNodeOffset + i] = Base::convertTo<SbVec3f>(pnt);
        }

        // normalize all normals
        for (int i = 0; i < numNorms; i++) {
            norms[i].normalize();
        }

        std::vector<int32_t> lineSetCoords;
        for (const auto& it : lineSetMap) {
            lineSetCoords.insert(lineSetCoords.end(), it.second.begin(), it.second.end());
            lineSetCoords.push_back(-1);
        }

        // preset the index vector size
        numLines = lineSetCoords.size();
        lineSet->coordIndex.setNum(numLines);
        int32_t* lines = lineSet->coordIndex.startEditing();

        int l = 0;
        for (auto it = lineSetCoords.begin(); it != lineSetCoords.end(); ++it, l++) {
            lines[l] = *it;
        }

        // end the editing of the nodes
        coords->point.finishEditing();
        norm->vector.finishEditing();
        faceSet->coordIndex.finishEditing();
        //faceSet->partIndex.finishEditing();
        lineSet->coordIndex.finishEditing();

        SoMaterial* material = new SoMaterial;
        material->diffuseColor.setValue(0.8, 0.8, 0.8);
        material->specularColor.setValue(1.0, 1.0, 1.0);
        material->shininess.setValue(0.5);

        shapeSep->addChild(material);
        shapeSep->addChild(normalBinding);
        shapeSep->addChild(norm);
        shapeSep->addChild(coords);
        shapeSep->addChild(faceSet);
        shapeSep->addChild(lineSet);
        shapeSep->addChild(pointSet);
        return shapeSep;

    }
    catch (const Standard_Failure& e) {
        //spdlog::error(e.GetMessageString());
        return nullptr;
    }
    catch (...) {
        //spdlog::error("Unknown error during shape conversion");
        return nullptr;
    }
}