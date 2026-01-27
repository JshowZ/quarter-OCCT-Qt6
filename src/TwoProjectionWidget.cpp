#include "TwoProjectionWidget.h"

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
#include <TopoDS_Vertex.hxx>
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

#include "base.h"

using namespace SIM::Coin3D::Quarter;

TwoProjectionWidget::TwoProjectionWidget(QWidget* parent)
    : QMainWindow(parent)
    , m_splitter(nullptr)
    , m_leftWidget(nullptr)
    , m_rightWidget(nullptr)
    , m_leftRoot(nullptr)
    , m_rightRoot(nullptr)
    , m_leftCamera(nullptr)
    , m_rightCamera(nullptr)
    , m_leftModelRoot(nullptr)
    , m_rightModelRoot(nullptr)
{
    setupUI();
    createOcctShapes();
    createScene();
    setupCameras();
    setupLeftModel();
    setupRightModel();
}

TwoProjectionWidget::~TwoProjectionWidget() {
    if (m_leftRoot) {
        m_leftRoot->unref();
    }
    if (m_rightRoot) {
        m_rightRoot->unref();
    }
}

void TwoProjectionWidget::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    
    m_splitter = new QSplitter(Qt::Horizontal, centralWidget);
    m_splitter->setSizes(QList<int>({400, 400}));
    
    m_leftWidget = new SIM::Coin3D::Quarter::QuarterWidget(m_splitter);
    m_rightWidget = new SIM::Coin3D::Quarter::QuarterWidget(m_splitter);
    
    m_splitter->addWidget(m_leftWidget);
    m_splitter->addWidget(m_rightWidget);
    
    centralLayout->addWidget(m_splitter);
    setCentralWidget(centralWidget);
    
    setWindowTitle("OCCT Two Projection Demo");
    resize(1000, 600);
}

void TwoProjectionWidget::createOcctShapes() {
    try {
        BRepPrimAPI_MakeBox leftMaker(2.0, 3.0, 1.5);
        leftMaker.Build();
        m_leftShape = leftMaker.Shape();

        BRepPrimAPI_MakeCylinder rightMaker(1.5, 4.0, 90.0);
        rightMaker.Build();
        m_rightShape = rightMaker.Shape();
    }
    catch (const Standard_Failure& e) {
        QMessageBox::critical(this, "OCCT Exception", e.GetMessageString());
    }
    catch (...) {
        QMessageBox::critical(this, "Error", "Unknown error creating OCCT shapes");
    }
}

SoSeparator* TwoProjectionWidget::createOcctModel(TopoDS_Shape shape) {
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

void TwoProjectionWidget::createScene() {
    m_leftRoot = new SoSeparator();
    m_leftRoot->ref();
    
    m_leftModelRoot = new SoSeparator();
    m_leftRoot->addChild(m_leftModelRoot);
    
    m_rightRoot = new SoSeparator();
    m_rightRoot->ref();
    
    m_rightModelRoot = new SoSeparator();
    m_rightRoot->addChild(m_rightModelRoot);
    
    SoDirectionalLight* leftLight = new SoDirectionalLight();
    leftLight->direction.setValue(1.0f, -1.0f, -1.0f);
    leftLight->intensity.setValue(0.8f);
    m_leftRoot->addChild(leftLight);
    
    SoDirectionalLight* rightLight = new SoDirectionalLight();
    rightLight->direction.setValue(1.0f, -1.0f, -1.0f);
    rightLight->intensity.setValue(0.8f);
    m_rightRoot->addChild(rightLight);
    
    m_leftWidget->setSceneGraph(m_leftRoot);
    m_rightWidget->setSceneGraph(m_rightRoot);
    
    m_leftWidget->setBackgroundColor(QColor::fromRgbF(1.0, 1.0, 1.0));
    m_rightWidget->setBackgroundColor(QColor::fromRgbF(1.0, 1.0, 1.0));
}

void TwoProjectionWidget::setupLeftModel() {
    SoSeparator* leftModel = createOcctModel(m_leftShape);
    if (leftModel) {
        m_leftModelRoot->addChild(leftModel);
    }
    m_leftWidget->viewAll();
}

void TwoProjectionWidget::setupRightModel() {
    SoSeparator* rightModel = createOcctModel(m_rightShape);
    if (rightModel) {
        m_rightModelRoot->addChild(rightModel);
    }
    m_rightWidget->viewAll();
}

void TwoProjectionWidget::setupCameras() {
    m_leftCamera = new SoPerspectiveCamera();
    m_leftCamera->position.setValue(8.0f, 6.0f, 8.0f);
    m_leftCamera->orientation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_leftCamera->nearDistance.setValue(0.1f);
    m_leftCamera->farDistance.setValue(1000.0f);
    m_leftCamera->heightAngle.setValue(M_PI / 4.0f);
    m_leftRoot->insertChild(m_leftCamera, 0);
    
    m_rightCamera = new SoOrthographicCamera();
    m_rightCamera->position.setValue(0.0f, 0.0f, 20.0f);
    m_rightCamera->orientation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_rightCamera->nearDistance.setValue(0.1f);
    m_rightCamera->farDistance.setValue(1000.0f);
    m_rightCamera->height.setValue(8.0f);
    m_rightRoot->insertChild(m_rightCamera, 0);
}
