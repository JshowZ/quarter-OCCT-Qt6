//#include <iostream>
//#include <string>
//
//#include <BRepMesh_IncrementalMesh.hxx> 
//#include <BRepTools.hxx>
//#include <STEPControl_Reader.hxx>
//#include <TopoDS_Shape.hxx>
//#include <TopoDS_Face.hxx>
//#include <TopoDS_Edge.hxx>
//#include <TopExp_Explorer.hxx>
//#include <TopoDS.hxx>
//#include <BRep_Tool.hxx>
//#include <GeomAdaptor_Curve.hxx>
//#include <GCPnts_TangentialDeflection.hxx>
//#include <TopExp.hxx>
//
//#include <Quarter/Quarter.h>
//#include <Quarter/QuarterWidget.h>
//
//#include <Inventor/VRMLnodes/SoVRMLCoordinate.h>
//#include <Inventor/VRMLnodes/SoVRMLGroup.h>
//#include <Inventor/VRMLnodes/SoVRMLIndexedFaceSet.h>
//#include <Inventor/VRMLnodes/SoVRMLMaterial.h>
//#include <Inventor/VRMLnodes/SoVRMLShape.h>
//#include <Inventor/actions/SoWriteAction.h>
//#include <Inventor/nodes/SoComplexity.h>
//#include <Inventor/nodes/SoDrawStyle.h>
//#include <Inventor/nodes/SoLightModel.h>
//#include <Inventor/nodes/SoMaterial.h>
//#include <Inventor/nodes/SoSeparator.h>
//#include <Inventor/nodes/SoCube.h>
//#include <Inventor/nodes/SoCoordinate3.h>
//#include <Inventor/nodes/SoLineSet.h>
//#include <Inventor/nodes/SoFaceSet.h>
//#include <Inventor/nodes/SoShapeHints.h>
//#include <Inventor/nodes/SoNormal.h>
//#include <Inventor/nodes/SoIndexedFaceSet.h>
//#include <Inventor/nodes/SoIndexedLineSet.h>
//#include <Inventor/nodes/SoPointSet.h>
//
//#include <QApplication>
//#include <QMainWindow>
//#include <QWidget>
//
//using namespace std;
//using namespace SIM::Coin3D::Quarter;
//
//
//// 辅助函数：获取三角网格的节点数
//int getTriangulationNodeCount(const Handle(Poly_Triangulation) & mesh)
//{
//    if (mesh.IsNull())
//        return 0;
//
//    return mesh->NbNodes();
//}
//
//
//gp_Pnt getTriangulationNode(const Handle(Poly_Triangulation) & mesh, int index)
//{
//    return mesh->Node(index + 1); // OCCT索引从1开始
//}
//
//
//int getTriangulationTriangleCount(const Handle(Poly_Triangulation) & mesh)
//{
//    if (mesh.IsNull())
//        return 0;
//
//    return mesh->NbTriangles();
//}
//
//void getTriangulationTriangle(const Handle(Poly_Triangulation) & mesh, int triIndex, int& n1, int& n2, int& n3)
//{
//    mesh->Triangle(triIndex + 1).Get(n1, n2, n3);
//}
//
//
//SoSeparator* convertOCCShapeToCoin(const TopoDS_Shape& shape)
//{
//    try {
//        SoSeparator* shapeSep = new SoSeparator;
//
//        // Create mesh for the entire shape with fine resolution
//        BRepMesh_IncrementalMesh mesh(shape, 0.1);
//        mesh.Perform();
//
//        if (!mesh.IsDone()) {
//            return nullptr;
//        }
//
//        // Explore all faces in the shape
//        TopExp_Explorer faceExplorer(shape, TopAbs_FACE);
//
//        // Create separate separators for faces and edges
//        SoSeparator* facesSep = new SoSeparator;
//        SoSeparator* edgesSep = new SoSeparator;
//
//        // Face rendering - use actual mesh data
//        while (faceExplorer.More()) {
//            TopoDS_Face face = TopoDS::Face(faceExplorer.Current());
//
//            // Get the mesh data for this face
//            TopLoc_Location loc;
//            Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, loc);
//
//            if (!triangulation.IsNull()) {
//                SoSeparator* faceSep = new SoSeparator;
//                SoCoordinate3* coords = new SoCoordinate3;
//                SoFaceSet* faceSet = new SoFaceSet;
//                SoMFInt32& numVertices = faceSet->numVertices;
//
//                // Add all vertices to coordinates
//                for (int i = 1; i <= triangulation->NbNodes(); ++i) {
//                    const gp_Pnt& pnt = triangulation->Node(i);
//                    coords->point.set1Value(i - 1, pnt.X(), pnt.Y(), pnt.Z());
//                }
//
//                // Add all triangles as faces
//                int faceIndex = 0;
//                for (int i = 1; i <= triangulation->NbTriangles(); ++i) {
//                    Poly_Triangle tri = triangulation->Triangle(i);
//                    Standard_Integer v1, v2, v3;
//                    tri.Get(v1, v2, v3);
//
//                    // Coin3D uses 0-based indexing, OCCT uses 1-based
//                    int idx1 = v1 - 1;
//                    int idx2 = v2 - 1;
//                    int idx3 = v3 - 1;
//
//                    // Define the triangle face
//                    numVertices.set1Value(faceIndex++, 3);
//                }
//
//                faceSep->addChild(coords);
//                faceSep->addChild(faceSet);
//                facesSep->addChild(faceSep);
//            }
//
//            faceExplorer.Next();
//        }
//
//        // Edge rendering for wireframe
//        TopExp_Explorer edgeExplorer(shape, TopAbs_EDGE);
//        while (edgeExplorer.More()) {
//            TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());
//            SoSeparator* edgeSep = new SoSeparator;
//
//            // Get edge curve
//            Standard_Real first, last;
//            Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
//
//            if (!curve.IsNull()) {
//                // Discretize the curve with finer resolution
//                GeomAdaptor_Curve curveAdaptor(curve);
//                GCPnts_TangentialDeflection discretizer;
//                discretizer.Initialize(curveAdaptor, 0.05, 0.05, first, last);
//
//                if (discretizer.NbPoints() > 0) {
//                    SoCoordinate3* coords = new SoCoordinate3;
//                    SoLineSet* lineSet = new SoLineSet;
//
//                    for (Standard_Integer i = 1; i <= discretizer.NbPoints(); ++i) {
//                        gp_Pnt pnt = discretizer.Value(i);
//                        coords->point.set1Value(i - 1, pnt.X(), pnt.Y(), pnt.Z());
//                    }
//
//                    edgeSep->addChild(coords);
//                    edgeSep->addChild(lineSet);
//                    edgesSep->addChild(edgeSep);
//                }
//            }
//
//            edgeExplorer.Next();
//        }
//
//        // Add faces and edges to the main separator
//        shapeSep->addChild(facesSep);
//        shapeSep->addChild(edgesSep);
//
//        return shapeSep;
//    }
//    catch (const Standard_Failure& e) {
//        return nullptr;
//    }
//    catch (...) {
//        return nullptr;
//    }
//}
//
//
//SoSeparator* convertShapeToCoin3D(const TopoDS_Shape& shape, double deflection = 0.01, double angularDeflection = 0.5)
//{
//    SoSeparator* root = new SoSeparator;
//    root->ref();
//
//    SoMaterial* material = new SoMaterial;
//    material->ambientColor.setValue(0.2, 0.2, 0.2);
//    material->diffuseColor.setValue(0.8, 0.8, 0.8);
//    material->specularColor.setValue(0.5, 0.5, 0.5);
//    material->shininess = 0.5;
//    root->addChild(material);
//
//    SoShapeHints* hints = new SoShapeHints;
//    hints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;
//    hints->shapeType = SoShapeHints::SOLID;
//    root->addChild(hints);
//
//    IMeshTools_Parameters meshParams;
//    meshParams.Deflection = deflection;
//    meshParams.Relative = false;
//    meshParams.Angle = angularDeflection * M_PI / 180.0;
//    meshParams.InParallel = true;
//
//    BRepMesh_IncrementalMesh mesher(shape, meshParams);
//
//    vector<SbVec3f> vertices;
//    vector<SbVec3f> normals;
//    vector<int32_t> faceIndices;
//    vector<int32_t> lineIndices;
//
//    TopLoc_Location location;
//
//    TopTools_IndexedMapOfShape faceMap;
//    TopExp::MapShapes(shape, TopAbs_FACE, faceMap);
//
//    for (int faceIdx = 1; faceIdx <= faceMap.Extent(); faceIdx++) {
//        const TopoDS_Face& face = TopoDS::Face(faceMap(faceIdx));
//
//        Handle(Poly_Triangulation) mesh = BRep_Tool::Triangulation(face, location);
//
//        if (mesh.IsNull())
//            continue;
//
//        int vertexOffset = vertices.size();
//        int nodeCount = getTriangulationNodeCount(mesh);
//        int triCount = getTriangulationTriangleCount(mesh);
//
//        for (int i = 0; i < nodeCount; i++) {
//            gp_Pnt pnt = getTriangulationNode(mesh, i);
//
//            if (!location.IsIdentity()) {
//                pnt.Transform(location.Transformation());
//            }
//
//            vertices.push_back(
//                SbVec3f(static_cast<float>(pnt.X()), static_cast<float>(pnt.Y()), static_cast<float>(pnt.Z())));
//
//            normals.push_back(SbVec3f(0, 0, 1));
//        }
//
//
//        const auto& triangles = mesh->Triangles();
//        for (int i = triangles.Lower(); i <= triangles.Upper(); i++) {
//            int n1, n2, n3;
//            triangles(i).Get(n1, n2, n3);
//
//            faceIndices.push_back(vertexOffset + n1 - 1);
//            faceIndices.push_back(vertexOffset + n2 - 1);
//            faceIndices.push_back(vertexOffset + n3 - 1);
//            faceIndices.push_back(-1); 
//        }
//
//        TopExp_Explorer edgeExp(face, TopAbs_EDGE);
//        while (edgeExp.More()) {
//            const TopoDS_Edge& edge = TopoDS::Edge(edgeExp.Current());
//
//            Handle(Poly_PolygonOnTriangulation) poly = BRep_Tool::PolygonOnTriangulation(edge, mesh, location);
//
//            if (!poly.IsNull()) {
//                const auto& indices = poly->Nodes();
//                for (int i = indices.Lower(); i <= indices.Upper(); i++) {
//                    lineIndices.push_back(vertexOffset + indices(i) - 1);
//                }
//                lineIndices.push_back(-1); 
//            }
//
//            edgeExp.Next();
//        }
//    }
//
//
//    TopTools_IndexedMapOfShape edgeMap;
//    TopExp::MapShapes(shape, TopAbs_EDGE, edgeMap);
//
//    set<int> faceEdges;
//    TopExp_Explorer faceExp(shape, TopAbs_FACE);
//    while (faceExp.More()) {
//        TopExp_Explorer edgeExp(faceExp.Current(), TopAbs_EDGE);
//        while (edgeExp.More()) {
//            // 记录属于面的边
//            int hash = TopTools_ShapeMapHasher{}(edgeExp.Current());
//            faceEdges.insert(hash);
//            edgeExp.Next();
//        }
//        faceExp.Next();
//    }
//
//    for (int edgeIdx = 1; edgeIdx <= edgeMap.Extent(); edgeIdx++) {
//        const TopoDS_Edge& edge = TopoDS::Edge(edgeMap(edgeIdx));
//
//        // 检查是否自由边
//        if (faceEdges.find(std::hash<TopoDS_Shape>{}(edge)) == faceEdges.end()) {
//            Handle(Poly_Polygon3D) poly = BRep_Tool::Polygon3D(edge, location);
//
//            if (!poly.IsNull()) {
//                int vertexOffset = vertices.size();
//
//                // 添加顶点
//                const auto& nodes = poly->Nodes();
//                for (int i = nodes.Lower(); i <= nodes.Upper(); i++) {
//                    gp_Pnt pnt = nodes(i);
//                    if (!location.IsIdentity()) {
//                        pnt.Transform(location.Transformation());
//                    }
//                    vertices.push_back(
//                        SbVec3f(static_cast<float>(pnt.X()), static_cast<float>(pnt.Y()), static_cast<float>(pnt.Z())));
//                }
//
//                // 添加线索引
//                for (int i = 0; i < nodes.Length(); i++) {
//                    lineIndices.push_back(vertexOffset + i);
//                }
//                lineIndices.push_back(-1); // 线结束标记
//            }
//        }
//    }
//
//    // 3. 处理顶点
//    TopTools_IndexedMapOfShape vertexMap;
//    TopExp::MapShapes(shape, TopAbs_VERTEX, vertexMap);
//
//    int pointOffset = vertices.size();
//    for (int vertIdx = 1; vertIdx <= vertexMap.Extent(); vertIdx++) {
//        const TopoDS_Vertex& vertex = TopoDS::Vertex(vertexMap(vertIdx));
//        gp_Pnt pnt = BRep_Tool::Pnt(vertex);
//        vertices.push_back(
//            SbVec3f(static_cast<float>(pnt.X()), static_cast<float>(pnt.Y()), static_cast<float>(pnt.Z())));
//    }
//
//    // 4. 创建Coin3D节点
//    if (!vertices.empty()) {
//        // 创建坐标节点
//        SoCoordinate3* coords = new SoCoordinate3;
//        coords->point.setValues(0, vertices.size(), vertices.data());
//        root->addChild(coords);
//
//        // 创建法线节点
//        SoNormal* norm = new SoNormal;
//        norm->vector.setValues(0, normals.size(), normals.data());
//        root->addChild(norm);
//
//        // 创建面节点
//        if (!faceIndices.empty()) {
//            SoIndexedFaceSet* faceSet = new SoIndexedFaceSet;
//            faceSet->coordIndex.setValues(0, faceIndices.size(), faceIndices.data());
//            root->addChild(faceSet);
//
//            cout << "面: " << faceIndices.size() / 4 << " 个三角形" << endl;
//        }
//
//        // 创建线节点
//        if (!lineIndices.empty()) {
//            SoDrawStyle* lineStyle = new SoDrawStyle;
//            lineStyle->style = SoDrawStyle::LINES;
//            lineStyle->lineWidth = 2.0;
//            root->addChild(lineStyle);
//
//            SoMaterial* lineMaterial = new SoMaterial;
//            lineMaterial->diffuseColor.setValue(0, 0, 0); // 黑色线条
//            root->addChild(lineMaterial);
//
//            SoIndexedLineSet* lineSet = new SoIndexedLineSet;
//            lineSet->coordIndex.setValues(0, lineIndices.size(), lineIndices.data());
//            root->addChild(lineSet);
//
//            cout << "线: " << lineIndices.size() << " 个索引" << endl;
//        }
//
//        // 创建点节点（显示顶点）
//        if (pointOffset < vertices.size()) {
//            SoDrawStyle* pointStyle = new SoDrawStyle;
//            pointStyle->style = SoDrawStyle::POINTS;
//            pointStyle->pointSize = 5.0;
//            root->addChild(pointStyle);
//
//            SoMaterial* pointMaterial = new SoMaterial;
//            pointMaterial->diffuseColor.setValue(1, 0, 0); // 红色点
//            root->addChild(pointMaterial);
//
//            SoPointSet* pointSet = new SoPointSet;
//            pointSet->startIndex.setValue(pointOffset);
//            pointSet->numPoints.setValue(vertexMap.Extent());
//            root->addChild(pointSet);
//
//            cout << "点: " << vertexMap.Extent() << " 个顶点" << endl;
//        }
//
//        cout << "总共: " << vertices.size() << " 个顶点" << endl;
//    }
//
//    return root;
//}
//
//
//bool readSTEPFile(const string& filename, TopoDS_Shape& shape)
//{
//    STEPControl_Reader reader;
//
//    IFSelect_ReturnStatus status = reader.ReadFile(filename.c_str());
//
//    if (status != IFSelect_RetDone) {
//        return false;
//    }
//
//    int nbroots = reader.NbRootsForTransfer();
//
//    reader.TransferRoots();
//
//    shape = reader.OneShape();
//
//    if (shape.IsNull()) {
//        return false;
//    }
//
//    TopAbs_ShapeEnum shapeType = shape.ShapeType();
//
//    switch (shapeType) {
//        case TopAbs_COMPOUND:
//            cout << " (COMPOUND)" << endl;
//            break;
//        case TopAbs_COMPSOLID:
//            cout << " (COMPSOLID)" << endl;
//            break;
//        case TopAbs_SOLID:
//            cout << "(SOLID)" << endl;
//            break;
//        case TopAbs_SHELL:
//            cout << " (SHELL)" << endl;
//            break;
//        case TopAbs_FACE:
//            cout << "(FACE)" << endl;
//            break;
//        case TopAbs_WIRE:
//            cout << " (WIRE)" << endl;
//            break;
//        case TopAbs_EDGE:
//            cout << " (EDGE)" << endl;
//            break;
//        case TopAbs_VERTEX:
//            cout << " (VERTEX)" << endl;
//            break;
//        case TopAbs_SHAPE:
//            cout << "(SHAPE)" << endl;
//            break;
//    }
//
//    return true;
//}
//
//int main(int argc, char** argv)
//{
//    QApplication app(argc, argv);
//
//    Quarter::init();
//
//    QMainWindow mainWindow;
//    mainWindow.setWindowTitle("OCCT + COIN3D/QUARTER STEP");
//    mainWindow.resize(800, 600);
//
//    string stepFilename = "D:/3DModels/Connectors/ALU-A_3.0x3.3x5.4.STEP";
//
//    TopoDS_Shape occShape;
//
//    if (!readSTEPFile(stepFilename, occShape)) 
//    {
//        std::cout << "Failed to read STEP file: " << stepFilename << std::endl;
//        return 1;
//    }
//
//    SoSeparator* coinRoot = convertShapeToCoin3D(occShape);
//
//    if (!coinRoot) {
//        return 1;
//    }
//    coinRoot->ref();
//
//    QuarterWidget* view = new QuarterWidget(&mainWindow);
//    view->setSceneGraph(coinRoot);
//
//    if (!view) 
//    {
//        return 1;
//    }
//
//    mainWindow.setCentralWidget(view);
//
//    mainWindow.show();
//
//    int result = app.exec();
//
//    coinRoot->unref();
//
//    return result;
//}


#include <iostream>
#include <QApplication>
#include <QSurfaceFormat>
#include <QFont>
#include <QDebug>

#include <Inventor/nodes/SoDepthBuffer.h>
#include <Inventor/SoDB.h>
#include <Inventor/SoInteraction.h>

#include <Quarter/Quarter.h>

#include "AnotherMainWindow.h"
#include "QuarterOcctViewer.h"
using namespace std;

int main(int argc, char** argv)
{
    // Set OpenGL format
     //QSurfaceFormat format;
     //format.setVersion(4, 1); // Set OpenGL version
     //format.setProfile(QSurfaceFormat::CoreProfile);
     //format.setSamples(4);    // Enable anti-aliasing
     //QSurfaceFormat::setDefaultFormat(format);
    
    // Create Qt application instance
     QApplication a(argc, argv);
    
    // Load Chinese font support
     QFont font = a.font();
     font.setFamily("SimHei"); // Use SimHei font
     a.setFont(font);
    
     try {
         // Create and show main window
         SoDB::init();
         SIM::Coin3D::Quarter::Quarter::init();
         //SoInteraction::init();
         //SoFCDB::init();
         QuarterOcctViewer w;
         w.show();
    
         // Enter application event loop
         return a.exec();
     } catch (const std::exception &e) {
         qDebug() << "Exception caught:" << e.what();
         return 1;
     } catch (...) {
         qDebug() << "Unknown exception caught";
         return 1;
     }
}