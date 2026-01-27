#include "ProjectionWgt.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>
#include <QMessageBox>

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoIndexedLineSet.h>
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/nodes/SoNormal.h>
#include <Inventor/nodes/SoNormalBinding.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoShapeHints.h>

#include <Quarter/QuarterWidget.h>

#include <TopoDS_Shape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <BRep_Tool.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopLoc_Location.hxx>
#include <Poly_Triangulation.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array1OfDir.hxx>
#include <Poly_Array1OfTriangle.hxx>
#include <Geom_Surface.hxx>
#include <Geom_Curve.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <TopoDS.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <IMeshTools_Parameters.hxx>
#include <Precision.hxx>
#include <gp.hxx>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepOffsetAPI_Sewing.hxx>
#include <BRepProj_Projection.hxx>
#include <gp_Pln.hxx>

#include "base.h"

using namespace SIM::Coin3D::Quarter;

ProjectionWgt::ProjectionWgt(QWidget* parent)
    : QMainWindow(parent)
    , m_splitter(nullptr)
    , m_originalWidget(nullptr)
    , m_projectionWidget(nullptr)
    , m_originalRoot(nullptr)
    , m_projectionRoot(nullptr)
    , m_originalCamera(nullptr)
    , m_projectionCamera(nullptr)
    , m_originalModelRoot(nullptr)
    , m_projectionModelRoot(nullptr)
{
    setupUI();
    createOriginalShape();
    createProjection();
    createScene();
    setupCameras();
    setupOriginalModel();
    setupProjectionModel();
}

ProjectionWgt::~ProjectionWgt() {
    if (m_originalRoot) {
        m_originalRoot->unref();
    }
    if (m_projectionRoot) {
        m_projectionRoot->unref();
    }
}

void ProjectionWgt::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    
    m_splitter = new QSplitter(Qt::Horizontal, centralWidget);
    m_splitter->setSizes(QList<int>({500, 500}));
    
    m_originalWidget = new SIM::Coin3D::Quarter::QuarterWidget(m_splitter);
    m_projectionWidget = new SIM::Coin3D::Quarter::QuarterWidget(m_splitter);
    
    m_splitter->addWidget(m_originalWidget);
    m_splitter->addWidget(m_projectionWidget);
    
    centralLayout->addWidget(m_splitter);
    setCentralWidget(centralWidget);
    
    setWindowTitle("OCCT Projection Demo");
    resize(1200, 600);
}

void ProjectionWgt::createOriginalShape() {
    try {
        BRepPrimAPI_MakeBox boxMaker(2.0, 3.0, 1.5);
        boxMaker.Build();
        m_originalShape = boxMaker.Shape();
    }
    catch (const Standard_Failure& e) {
        QMessageBox::critical(this, "OCCT Exception", e.GetMessageString());
    }
    catch (...) {
        QMessageBox::critical(this, "Error", "Unknown error creating original shape");
    }
}

void ProjectionWgt::createProjection() {
    try {
        // Create projection plane (XY plane)
        gp_Pln projectionPlane(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
        TopoDS_Face projectionFace = BRepBuilderAPI_MakeFace(projectionPlane, -5.0, 5.0, -5.0, 5.0).Face();
        
        // Create projection direction (along Z-axis)
        gp_Dir projectionDir(0, 0, 1);
        
        // Create projected shape
        BRep_Builder builder;
        TopoDS_Compound projectedCompound;
        builder.MakeCompound(projectedCompound);
        
        // Project each edge of original shape onto the plane
        TopExp_Explorer edgeExplorer(m_originalShape, TopAbs_EDGE);
        while (edgeExplorer.More()) {
            const TopoDS_Edge& edge = TopoDS::Edge(edgeExplorer.Current());
            
            BRepProj_Projection projector(edge, projectionFace, projectionDir);
            if (projector.IsDone()) {
                while (projector.More()) {
                    const TopoDS_Wire& projectedWire = projector.Current();
                    builder.Add(projectedCompound, projectedWire);
                    projector.Next();
                }
            }
            
            edgeExplorer.Next();
        }
        
        m_projectionShape = projectedCompound;
    }
    catch (const Standard_Failure& e) {
        QMessageBox::critical(this, "OCCT Exception", e.GetMessageString());
    }
    catch (...) {
        QMessageBox::critical(this, "Error", "Unknown error creating projection");
    }
}

SoSeparator* ProjectionWgt::createOcctModel(TopoDS_Shape shape) {
    SoSeparator* shapeSep = new SoSeparator;
    
    if (shape.IsNull()) {
        return shapeSep;
    }
    
    Standard_Real deflection = 0.1;
    if (deflection < gp::Resolution()) {
        deflection = Precision::Confusion();
    }
    
    IMeshTools_Parameters meshParams;
    meshParams.Deflection = deflection;
    meshParams.Relative = Standard_False;
    meshParams.Angle = 0.5;
    meshParams.InParallel = Standard_True;
    meshParams.AllowQualityDecrease = Standard_True;
    
    BRepMesh_IncrementalMesh meshGenerator(shape, meshParams);
    
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(shape, TopAbs_FACE, faceMap);
    
    int numTriangles = 0, numNodes = 0, numNorms = 0;
    
    TopLoc_Location aLoc;
    shape.Location(aLoc);
    
    for (int i = 1; i <= faceMap.Extent(); i++) {
        const TopoDS_Face& face = TopoDS::Face(faceMap(i));
        Handle(Poly_Triangulation) mesh = BRep_Tool::Triangulation(face, aLoc);
        
        if (!mesh.IsNull()) {
            numTriangles += mesh->NbTriangles();
            numNodes += mesh->NbNodes();
            numNorms += mesh->NbNodes();
        }
    }
    
    TopTools_IndexedMapOfShape edgeMap;
    TopExp::MapShapes(shape, TopAbs_EDGE, edgeMap);
    
    int edgeNodes = 0;
    for (int i = 1; i <= edgeMap.Extent(); i++) {
        const TopoDS_Edge& aEdge = TopoDS::Edge(edgeMap(i));
        
        Standard_Real first, last;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(aEdge, first, last);
        
        if (!curve.IsNull()) {
            GeomAdaptor_Curve curveAdaptor(curve);
            GCPnts_TangentialDeflection discretizer;
            discretizer.Initialize(curveAdaptor, 0.05, 0.05, first, last);
            edgeNodes += discretizer.NbPoints();
        }
    }
    numNodes += edgeNodes;
    
    TopTools_IndexedMapOfShape vertexMap;
    TopExp::MapShapes(shape, TopAbs_VERTEX, vertexMap);
    numNodes += vertexMap.Extent();
    
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
    
    normalBinding->value.setValue(SoNormalBinding::PER_VERTEX_INDEXED);
    
    coords->point.setNum(numNodes);
    norm->vector.setNum(numNorms);
    faceSet->coordIndex.setNum(numTriangles * 4);
    
    SbVec3f* verts = coords->point.startEditing();
    SbVec3f* norms = norm->vector.startEditing();
    int32_t* index = faceSet->coordIndex.startEditing();
    
    for (int i = 0; i < numNorms; i++) {
        norms[i] = SbVec3f(0.0, 0.0, 0.0);
    }
    
    int faceNodeOffset = 0, faceTriaOffset = 0;
    
    for (int i = 1; i <= faceMap.Extent(); i++) {
        TopLoc_Location faceLoc;
        const TopoDS_Face& actFace = TopoDS::Face(faceMap(i));
        
        Handle(Poly_Triangulation) mesh = BRep_Tool::Triangulation(actFace, faceLoc);
        if (mesh.IsNull()) {
            continue;
        }
        
        gp_Trsf myTransf;
        Standard_Boolean identity = true;
        if (!faceLoc.IsIdentity()) {
            identity = false;
            myTransf = faceLoc.Transformation();
        }
        
        int nbNodesInFace = mesh->NbNodes();
        int nbTriInFace = mesh->NbTriangles();
        TopAbs_Orientation orient = actFace.Orientation();
        
        for (int g = 1; g <= nbTriInFace; g++) {
            Standard_Integer N1, N2, N3;
            mesh->Triangle(g).Get(N1, N2, N3);
            
            if (orient != TopAbs_FORWARD) {
                Standard_Integer tmp = N1;
                N1 = N2;
                N2 = tmp;
            }
            
            gp_Pnt V1(mesh->Node(N1)), V2(mesh->Node(N2)), V3(mesh->Node(N3));
            
            if (!identity) {
                V1.Transform(myTransf);
                V2.Transform(myTransf);
                V3.Transform(myTransf);
            }
            
            gp_Vec v1 = V2.XYZ() - V1.XYZ();
            gp_Vec v2 = V3.XYZ() - V1.XYZ();
            gp_Vec triangleNormal = v1 ^ v2;
            
            if (triangleNormal.SquareMagnitude() > Precision::SquareConfusion()) {
                triangleNormal.Normalize();
            } else {
                triangleNormal.SetXYZ(gp_XYZ(0.0, 0.0, 1.0));
            }
            
            int idx1 = faceNodeOffset + N1 - 1;
            int idx2 = faceNodeOffset + N2 - 1;
            int idx3 = faceNodeOffset + N3 - 1;
            
            verts[idx1] = SbVec3f(V1.X(), V1.Y(), V1.Z());
            verts[idx2] = SbVec3f(V2.X(), V2.Y(), V2.Z());
            verts[idx3] = SbVec3f(V3.X(), V3.Y(), V3.Z());
            
            norms[idx1] += SbVec3f(triangleNormal.X(), triangleNormal.Y(), triangleNormal.Z());
            norms[idx2] += SbVec3f(triangleNormal.X(), triangleNormal.Y(), triangleNormal.Z());
            norms[idx3] += SbVec3f(triangleNormal.X(), triangleNormal.Y(), triangleNormal.Z());
            
            int triIndex = faceTriaOffset * 4;
            index[triIndex] = idx1;
            index[triIndex + 1] = idx2;
            index[triIndex + 2] = idx3;
            index[triIndex + 3] = SO_END_FACE_INDEX;
        }
        
        faceNodeOffset += nbNodesInFace;
        faceTriaOffset += nbTriInFace;
    }
    
    for (int i = 0; i < numNorms; i++) {
        if (norms[i].sqrLength() > 1e-12) {
            norms[i].normalize();
        } else {
            norms[i] = SbVec3f(0.0f, 0.0f, 1.0f);
        }
    }
    
    std::vector<int32_t> lineSetCoords;
    int edgeNodeOffset = faceNodeOffset;
    
    for (int i = 1; i <= edgeMap.Extent(); i++) {
        const TopoDS_Edge& aEdge = TopoDS::Edge(edgeMap(i));
        
        Standard_Real first, last;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(aEdge, first, last);
        
        if (!curve.IsNull()) {
            GeomAdaptor_Curve curveAdaptor(curve);
            GCPnts_TangentialDeflection discretizer;
            discretizer.Initialize(curveAdaptor, 0.05, 0.05, first, last);
            
            if (discretizer.NbPoints() > 0) {
                for (Standard_Integer j = 1; j <= discretizer.NbPoints(); j++) {
                    gp_Pnt pnt = discretizer.Value(j);
                    if (!aEdge.Location().IsIdentity()) {
                        pnt.Transform(aEdge.Location().Transformation());
                    }
                    
                    int nodeIndex = edgeNodeOffset++;
                    verts[nodeIndex] = SbVec3f(pnt.X(), pnt.Y(), pnt.Z());
                    lineSetCoords.push_back(nodeIndex);
                }
                lineSetCoords.push_back(-1);
            }
        }
    }
    
    pointSet->startIndex.setValue(edgeNodeOffset);
    for (int i = 0; i < vertexMap.Extent(); i++) {
        const TopoDS_Vertex& aVertex = TopoDS::Vertex(vertexMap(i + 1));
        gp_Pnt pnt = BRep_Tool::Pnt(aVertex);
        
        if (!aVertex.Location().IsIdentity()) {
            pnt.Transform(aVertex.Location().Transformation());
        }
        
        verts[edgeNodeOffset + i] = SbVec3f(pnt.X(), pnt.Y(), pnt.Z());
    }
    
    lineSet->coordIndex.setNum(lineSetCoords.size());
    int32_t* lines = lineSet->coordIndex.startEditing();
    
    for (size_t i = 0; i < lineSetCoords.size(); i++) {
        lines[i] = lineSetCoords[i];
    }
    
    coords->point.finishEditing();
    norm->vector.finishEditing();
    faceSet->coordIndex.finishEditing();
    lineSet->coordIndex.finishEditing();
    
    SoMaterial* material = new SoMaterial;
    material->diffuseColor.setValue(0.5f, 0.7f, 0.9f);
    material->specularColor.setValue(0.8f, 0.8f, 0.8f);
    material->shininess.setValue(0.4f);
    
    SoDrawStyle* drawStyle = new SoDrawStyle;
    drawStyle->style.setValue(SoDrawStyle::FILLED);
    
    SoShapeHints* shapeHints = new SoShapeHints;
    shapeHints->vertexOrdering.setValue(SoShapeHints::COUNTERCLOCKWISE);
    
    shapeSep->addChild(shapeHints);
    shapeSep->addChild(drawStyle);
    shapeSep->addChild(material);
    shapeSep->addChild(normalBinding);
    shapeSep->addChild(norm);
    shapeSep->addChild(coords);
    shapeSep->addChild(faceSet);
    shapeSep->addChild(lineSet);
    shapeSep->addChild(pointSet);
    
    return shapeSep;
}

void ProjectionWgt::createScene() {
    m_originalRoot = new SoSeparator();
    m_originalRoot->ref();
    
    m_originalModelRoot = new SoSeparator();
    m_originalRoot->addChild(m_originalModelRoot);
    
    m_projectionRoot = new SoSeparator();
    m_projectionRoot->ref();
    
    m_projectionModelRoot = new SoSeparator();
    m_projectionRoot->addChild(m_projectionModelRoot);
    
    SoDirectionalLight* originalLight = new SoDirectionalLight();
    originalLight->direction.setValue(1.0f, -1.0f, -1.0f);
    originalLight->intensity.setValue(0.8f);
    m_originalRoot->addChild(originalLight);
    
    SoDirectionalLight* projectionLight = new SoDirectionalLight();
    projectionLight->direction.setValue(1.0f, -1.0f, -1.0f);
    projectionLight->intensity.setValue(0.8f);
    m_projectionRoot->addChild(projectionLight);
    
    m_originalWidget->setSceneGraph(m_originalRoot);
    m_projectionWidget->setSceneGraph(m_projectionRoot);
    
    m_originalWidget->setBackgroundColor(QColor::fromRgbF(1.0, 1.0, 1.0));
    m_projectionWidget->setBackgroundColor(QColor::fromRgbF(1.0, 1.0, 1.0));
}

void ProjectionWgt::setupOriginalModel() {
    SoSeparator* originalModel = createOcctModel(m_originalShape);
    if (originalModel) {
        m_originalModelRoot->addChild(originalModel);
    }
    m_originalWidget->viewAll();
}

void ProjectionWgt::setupProjectionModel() {
    SoSeparator* projectionModel = createOcctModel(m_projectionShape);
    if (projectionModel) {
        m_projectionModelRoot->addChild(projectionModel);
    }
    m_projectionWidget->viewAll();
}

void ProjectionWgt::setupCameras() {
    m_originalCamera = new SoPerspectiveCamera();
    m_originalCamera->position.setValue(8.0f, 6.0f, 8.0f);
    m_originalCamera->orientation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_originalCamera->nearDistance.setValue(0.1f);
    m_originalCamera->farDistance.setValue(1000.0f);
    m_originalCamera->heightAngle.setValue(M_PI / 4.0f);
    m_originalRoot->insertChild(m_originalCamera, 0);
    
    m_projectionCamera = new SoOrthographicCamera();
    m_projectionCamera->position.setValue(0.0f, 0.0f, 20.0f);
    m_projectionCamera->orientation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_projectionCamera->nearDistance.setValue(0.1f);
    m_projectionCamera->farDistance.setValue(1000.0f);
    m_projectionCamera->height.setValue(8.0f);
    m_projectionRoot->insertChild(m_projectionCamera, 0);
}
