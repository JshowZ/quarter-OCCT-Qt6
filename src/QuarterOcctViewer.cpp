#include "QuarterOcctViewer.h"

// Qt headers
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSlider>
#include <QPushButton>
#include <QWheelEvent>

// Coin3D and Quarter headers
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoFaceSet.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoNormal.h>
#include <Inventor/nodes/SoNormalBinding.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoIndexedLineSet.h>
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/SoInteraction.h>
#include <Quarter/QuarterWidget.h>

// Use Quarter namespace for convenience
using namespace SIM::Coin3D::Quarter;

// OCCT headers for model reading
#include <STEPControl_Reader.hxx>
#include <IGESControl_Reader.hxx>
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
#include <Geom_Point.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <TopoDS.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Polygon3D.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <IMeshTools_Parameters.hxx>
#include <Precision.hxx>
#include <gp.hxx>

QuarterOcctViewer::QuarterOcctViewer(QWidget *parent)
    : QMainWindow(parent),
      m_centralWidget(nullptr),
      m_mainLayout(nullptr),
      m_quarterWidget(nullptr),
      m_root(nullptr),
      m_modelRoot(nullptr),
      m_zoomSlider(nullptr),
      m_wireframeMode(false)
{
    setupUI();
}

QuarterOcctViewer::~QuarterOcctViewer()
{
    // Cleanup Coin3D nodes
    if (m_root) {
        m_root->unref();
    }
}

void QuarterOcctViewer::setupUI()
{
    // Create central widget and main layout
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Create Quarter widget for 3D rendering
    m_quarterWidget = new SIM::Coin3D::Quarter::QuarterWidget(m_centralWidget);
    m_mainLayout->addWidget(m_quarterWidget);
    
    // Create control panel
    QWidget* controlPanel = new QWidget(m_centralWidget);
    QHBoxLayout* controlLayout = new QHBoxLayout(controlPanel);
    controlLayout->setContentsMargins(5, 5, 5, 5);
    
    // Add wireframe checkbox
    QCheckBox* wireframeCheckBox = new QCheckBox("Wireframe", controlPanel);
    connect(wireframeCheckBox, &QCheckBox::stateChanged, this, [this](int state) {
        setWireframe(state == Qt::Checked);
    });
    controlLayout->addWidget(wireframeCheckBox);
    
    // Add zoom slider
    controlLayout->addWidget(new QLabel("Zoom:", controlPanel));
    m_zoomSlider = new QSlider(Qt::Horizontal, controlPanel);
    m_zoomSlider->setRange(1, 100);
    m_zoomSlider->setValue(50);
    connect(m_zoomSlider, &QSlider::valueChanged, this, &QuarterOcctViewer::onZoomChanged);
    controlLayout->addWidget(m_zoomSlider);
    
    // Add zoom all button
    QAction* zoomAllAction = new QAction("Zoom All", controlPanel);
    connect(zoomAllAction, &QAction::triggered, this, &QuarterOcctViewer::zoomAll);
    
    QPushButton* zoomAllButton = new QPushButton("Zoom All", controlPanel);
    connect(zoomAllButton, &QPushButton::clicked, this, &QuarterOcctViewer::zoomAll);
    controlLayout->addWidget(zoomAllButton);
    
    m_mainLayout->addWidget(controlPanel);
    m_mainLayout->setStretch(0, 1); // Make quarter widget expand
    
    // Create menu bar
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // File menu
    QMenu* fileMenu = menuBar->addMenu("File");
    
    QAction* openStepAction = fileMenu->addAction("Open STEP File");
    connect(openStepAction, &QAction::triggered, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(
            this, "Open STEP File", "", "STEP Files (*.step *.stp)");
        if (!filePath.isEmpty()) {
            loadSTEPFile(filePath.toStdString());
        }
    });
    
    QAction* openIgesAction = fileMenu->addAction("Open IGES File");
    connect(openIgesAction, &QAction::triggered, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(
            this, "Open IGES File", "", "IGES Files (*.iges *.igs)");
        if (!filePath.isEmpty()) {
            loadIGESFile(filePath.toStdString());
        }
    });
    
    fileMenu->addSeparator();
    
    QAction* exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    
    // View menu
    QMenu* viewMenu = menuBar->addMenu("View");
    
    QAction* wireframeAction = viewMenu->addAction("Wireframe");
    wireframeAction->setCheckable(true);
    connect(wireframeAction, &QAction::triggered, this, [this](bool checked) {
        setWireframe(checked);
        //wireframeCheckBox->setChecked(checked);
    });
    
    viewMenu->addAction(zoomAllAction);
    
    // Help menu
    QMenu* helpMenu = menuBar->addMenu("Help");
    
    QAction* aboutAction = helpMenu->addAction("About");
    connect(aboutAction, &QAction::triggered, this, &QuarterOcctViewer::onAbout);
    
    // Initialize Coin3D interaction system (for mouse rotation, pan, zoom)
    SoInteraction::init();
    
    // Initialize Coin3D scene
    m_root = new SoSeparator;
    m_root->ref();
    
    // Add camera
    m_camera = new SoPerspectiveCamera;
    m_camera->position.setValue(0, 0, 10);
    m_camera->heightAngle.setValue(M_PI / 4.0);
    m_camera->nearDistance.setValue(0.1);
    m_camera->farDistance.setValue(1000.0);
    m_root->addChild(m_camera);
    
    // Add light
    SoDirectionalLight* light = new SoDirectionalLight;
    light->direction.setValue(-0.5, -0.5, -1);
    m_root->addChild(light);
    
    // Add shape hints for better rendering
    SoShapeHints* shapeHints = new SoShapeHints;
    shapeHints->vertexOrdering.setValue(SoShapeHints::COUNTERCLOCKWISE);
    m_root->addChild(shapeHints);
    
    // Add draw style node for wireframe control
    SoDrawStyle* drawStyle = new SoDrawStyle;
    drawStyle->style.setValue(SoDrawStyle::FILLED);
    m_root->addChild(drawStyle);
    
    // Add material
    SoMaterial* material = new SoMaterial;
    material->diffuseColor.setValue(0.8, 0.8, 0.8);
    m_root->addChild(material);
    
    // Add model root separator
    m_modelRoot = new SoSeparator;
    m_root->addChild(m_modelRoot);
    
    // Set the scene graph to Quarter widget
    m_quarterWidget->setSceneGraph(m_root);
    
    // Set default background color to white
        setBackgroundColor(1.0, 1.0, 1.0);
        
        // Set window properties
        setWindowTitle("Quarter OCCT Viewer");
        resize(800, 600);
}

SoNode* QuarterOcctViewer::convertOcctShapeToCoin3D(const TopoDS_Shape& shape)
{
    try {
        SoSeparator* shapeSep = new SoSeparator;
        
        if (shape.IsNull()) {
            return shapeSep;
        }
        
        // Create mesh for the entire shape with appropriate resolution
        Standard_Real deflection = 0.1; // Adjust this value for better quality vs performance
        
        // Since OCCT 7.6 a value of equal 0 is not allowed any more
        if (deflection < gp::Resolution()) {
            deflection = Precision::Confusion();
        }
        
        // For very big objects the computed deflection can become very high
        deflection = std::min(deflection, 20.0);
        
        // Create mesh parameters
        IMeshTools_Parameters meshParams;
        meshParams.Deflection = deflection;
        meshParams.Relative = Standard_False;
        meshParams.Angle = 0.5; // Angular deflection in radians
        meshParams.InParallel = Standard_True;
        meshParams.AllowQualityDecrease = Standard_True;
        
        // Generate mesh
        BRepMesh_IncrementalMesh meshGenerator(shape, meshParams);
        
        // First pass: Calculate total number of nodes needed
        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(shape, TopAbs_FACE, faceMap);
        
        int numTriangles = 0, numNodes = 0, numNorms = 0, numFaces = 0, numEdges = 0;
        
        // Calculate nodes from faces
        for (int i = 1; i <= faceMap.Extent(); i++) {
            TopLoc_Location faceLoc;
            Handle(Poly_Triangulation) mesh = BRep_Tool::Triangulation(TopoDS::Face(faceMap(i)), faceLoc);
            
            if (!mesh.IsNull()) {
                numTriangles += mesh->NbTriangles();
                numNodes += mesh->NbNodes();
                numNorms += mesh->NbNodes();
            }
            numFaces++;
        }
        
        // Calculate nodes from edges
        TopTools_IndexedMapOfShape edgeMap;
        TopExp::MapShapes(shape, TopAbs_EDGE, edgeMap);
        numEdges = edgeMap.Extent();
        
        int edgeNodes = 0;
        for (int i = 1; i <= edgeMap.Extent(); i++) {
            const TopoDS_Edge& aEdge = TopoDS::Edge(edgeMap(i));
            
            // Get edge curve
            Standard_Real first, last;
            Handle(Geom_Curve) curve = BRep_Tool::Curve(aEdge, first, last);
            
            if (!curve.IsNull()) {
                // Discretize the curve to get point count
                GeomAdaptor_Curve curveAdaptor(curve);
                GCPnts_TangentialDeflection discretizer;
                discretizer.Initialize(curveAdaptor, 0.05, 0.05, first, last);
                
                edgeNodes += discretizer.NbPoints();
            }
        }
        numNodes += edgeNodes;
        
        // Calculate nodes from vertices
        TopTools_IndexedMapOfShape vertexMap;
        TopExp::MapShapes(shape, TopAbs_VERTEX, vertexMap);
        numNodes += vertexMap.Extent();
        
        // Create Coin3D nodes with correct sizes
        SoCoordinate3* coords = new SoCoordinate3;
        SoNormal* norm = new SoNormal;
        SoNormalBinding* normalBinding = new SoNormalBinding;
        SoIndexedFaceSet* faceSet = new SoIndexedFaceSet;
        SoIndexedLineSet* lineSet = new SoIndexedLineSet;
        SoPointSet* pointSet = new SoPointSet;
        
        normalBinding->value.setValue(SoNormalBinding::PER_VERTEX_INDEXED);
        
        // Allocate memory for nodes and indexes
        coords->point.setNum(numNodes);
        norm->vector.setNum(numNorms);
        faceSet->coordIndex.setNum(numTriangles * 4); // 3 vertices + SO_END_FACE_INDEX
        
        // Get raw pointers for fast access
        SbVec3f* verts = coords->point.startEditing();
        SbVec3f* norms = norm->vector.startEditing();
        int32_t* index = faceSet->coordIndex.startEditing();
        
        // Initialize normals to zero vector
        for (int i = 0; i < numNorms; i++) {
            norms[i] = SbVec3f(0.0, 0.0, 0.0);
        }
        
        // Process faces (second pass)
        int faceNodeOffset = 0, faceTriaOffset = 0;
        
        for (int i = 1; i <= faceMap.Extent(); i++) {
            TopLoc_Location faceLoc;
            const TopoDS_Face& actFace = TopoDS::Face(faceMap(i));
            
            Handle(Poly_Triangulation) mesh = BRep_Tool::Triangulation(actFace, faceLoc);
            if (mesh.IsNull()) {
                continue;
            }
            
            // Get transformation
            gp_Trsf myTransf;
            Standard_Boolean identity = true;
            if (!faceLoc.IsIdentity()) {
                identity = false;
                myTransf = faceLoc.Transformation();
            }
            
            int nbNodesInFace = mesh->NbNodes();
            int nbTriInFace = mesh->NbTriangles();
            TopAbs_Orientation orient = actFace.Orientation();
            
            // Process triangles
            for (int g = 1; g <= nbTriInFace; g++) {
                Standard_Integer N1, N2, N3;
                mesh->Triangle(g).Get(N1, N2, N3);
                
                // Adjust orientation if needed
                if (orient != TopAbs_FORWARD) {
                    Standard_Integer tmp = N1;
                    N1 = N2;
                    N2 = tmp;
                }
                
                // Get vertices
                gp_Pnt V1(mesh->Node(N1)), V2(mesh->Node(N2)), V3(mesh->Node(N3));
                
                // Apply transformation
                if (!identity) {
                    V1.Transform(myTransf);
                    V2.Transform(myTransf);
                    V3.Transform(myTransf);
                }
                
                // Calculate normal
                gp_Vec v1 = V2.XYZ() - V1.XYZ();
                gp_Vec v2 = V3.XYZ() - V1.XYZ();
                gp_Vec triangleNormal = v1 ^ v2;
                
                // Normalize if non-zero
                if (triangleNormal.SquareMagnitude() > Precision::SquareConfusion()) {
                    triangleNormal.Normalize();
                } else {
                    triangleNormal.SetXYZ(gp_XYZ(0.0, 0.0, 1.0));
                }
                
                // Store vertex coordinates
                int idx1 = faceNodeOffset + N1 - 1;
                int idx2 = faceNodeOffset + N2 - 1;
                int idx3 = faceNodeOffset + N3 - 1;
                
                verts[idx1] = SbVec3f(V1.X(), V1.Y(), V1.Z());
                verts[idx2] = SbVec3f(V2.X(), V2.Y(), V2.Z());
                verts[idx3] = SbVec3f(V3.X(), V3.Y(), V3.Z());
                
                // Accumulate normals
                norms[idx1] += SbVec3f(triangleNormal.X(), triangleNormal.Y(), triangleNormal.Z());
                norms[idx2] += SbVec3f(triangleNormal.X(), triangleNormal.Y(), triangleNormal.Z());
                norms[idx3] += SbVec3f(triangleNormal.X(), triangleNormal.Y(), triangleNormal.Z());
                
                // Set face index
                int triIndex = faceTriaOffset * 4;
                index[triIndex] = idx1;
                index[triIndex + 1] = idx2;
                index[triIndex + 2] = idx3;
                index[triIndex + 3] = SO_END_FACE_INDEX;
            }
            
            // Update offsets
            faceNodeOffset += nbNodesInFace;
            faceTriaOffset += nbTriInFace;
        }
        
        // Normalize all normals
        for (int i = 0; i < numNorms; i++) {
            if (norms[i].sqrLength() > 1e-12) {
                norms[i].normalize();
            } else {
                norms[i] = SbVec3f(0.0f, 0.0f, 1.0f);
            }
        }
        
        // Process edges (third pass)
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
                    // Store edge vertices
                    for (Standard_Integer j = 1; j <= discretizer.NbPoints(); j++) {
                        gp_Pnt pnt = discretizer.Value(j);
                        if (!aEdge.Location().IsIdentity()) {
                            pnt.Transform(aEdge.Location().Transformation());
                        }
                        
                        int nodeIndex = edgeNodeOffset++;
                        verts[nodeIndex] = SbVec3f(pnt.X(), pnt.Y(), pnt.Z());
                        lineSetCoords.push_back(nodeIndex);
                    }
                    lineSetCoords.push_back(-1); // End marker
                }
            }
        }
        
        // Process vertices (fourth pass)
        pointSet->startIndex.setValue(edgeNodeOffset);
        for (int i = 0; i < vertexMap.Extent(); i++) {
            const TopoDS_Vertex& aVertex = TopoDS::Vertex(vertexMap(i + 1));
            gp_Pnt pnt = BRep_Tool::Pnt(aVertex);
            
            if (!aVertex.Location().IsIdentity()) {
                pnt.Transform(aVertex.Location().Transformation());
            }
            
            verts[edgeNodeOffset + i] = SbVec3f(pnt.X(), pnt.Y(), pnt.Z());
        }
        
        // Set line set coordinates
        lineSet->coordIndex.setNum(lineSetCoords.size());
        int32_t* lines = lineSet->coordIndex.startEditing();
        
        for (size_t i = 0; i < lineSetCoords.size(); i++) {
            lines[i] = lineSetCoords[i];
        }
        
        // Finish editing all nodes
        coords->point.finishEditing();
        norm->vector.finishEditing();
        faceSet->coordIndex.finishEditing();
        lineSet->coordIndex.finishEditing();
        
        // Add material for better visualization
        SoMaterial* material = new SoMaterial;
        material->diffuseColor.setValue(0.8, 0.8, 0.8);
        material->specularColor.setValue(1.0, 1.0, 1.0);
        material->shininess.setValue(0.5);
        
        // Assemble the scene graph
        shapeSep->addChild(material);
        shapeSep->addChild(normalBinding);
        shapeSep->addChild(norm);
        shapeSep->addChild(coords);
        shapeSep->addChild(faceSet);
        shapeSep->addChild(lineSet);
        shapeSep->addChild(pointSet);
        
        return shapeSep;
    } catch (const Standard_Failure& e) {
        QMessageBox::critical(this, "OCCT Exception", e.GetMessageString());
        return nullptr;
    } catch (...) {
        QMessageBox::critical(this, "Error", "Unknown error during shape conversion");
        return nullptr;
    }
}

bool QuarterOcctViewer::readSTEPFile(const std::string& filePath, TopoDS_Shape& shape)
{
    try {
        STEPControl_Reader reader;
        IFSelect_ReturnStatus status = reader.ReadFile(filePath.c_str());
        
        if (status != IFSelect_RetDone) {
            QMessageBox::critical(this, "Error", "Failed to read STEP file");
            return false;
        }
        
        reader.TransferRoots();
        shape = reader.OneShape();
        
        if (shape.IsNull()) {
            QMessageBox::critical(this, "Error", "No shapes found in STEP file");
            return false;
        }
        
        return true;
    } catch (const Standard_Failure& e) {
        QMessageBox::critical(this, "OCCT Exception", e.GetMessageString());
        return false;
    } catch (...) {
        QMessageBox::critical(this, "Error", "Unknown error during STEP file reading");
        return false;
    }
}

bool QuarterOcctViewer::readIGESFile(const std::string& filePath, TopoDS_Shape& shape)
{
    try {
        IGESControl_Reader reader;
        IFSelect_ReturnStatus status = reader.ReadFile(filePath.c_str());
        
        if (status != IFSelect_RetDone) {
            QMessageBox::critical(this, "Error", "Failed to read IGES file");
            return false;
        }
        
        reader.TransferRoots();
        shape = reader.OneShape();
        
        if (shape.IsNull()) {
            QMessageBox::critical(this, "Error", "No shapes found in IGES file");
            return false;
        }
        
        return true;
    } catch (const Standard_Failure& e) {
        QMessageBox::critical(this, "OCCT Exception", e.GetMessageString());
        return false;
    } catch (...) {
        QMessageBox::critical(this, "Error", "Unknown error during IGES file reading");
        return false;
    }
}

bool QuarterOcctViewer::loadSTEPFile(const std::string& filePath)
{
    TopoDS_Shape shape;
    if (!readSTEPFile(filePath, shape)) {
        return false;
    }
    
    return displayShape(shape);
}

bool QuarterOcctViewer::loadIGESFile(const std::string& filePath)
{
    TopoDS_Shape shape;
    if (!readIGESFile(filePath, shape)) {
        return false;
    }
    
    return displayShape(shape);
}

bool QuarterOcctViewer::displayShape(const TopoDS_Shape& shape)
{
    // Clear existing model
    clearScene();
    
    // Convert OCCT shape to Coin3D node
    SoNode* modelNode = convertOcctShapeToCoin3D(shape);
    if (modelNode) {
        m_modelRoot->addChild(modelNode);
        
        // Zoom to fit
        zoomAll();
        
        return true;
    }
    
    return false;
}

void QuarterOcctViewer::clearScene()
{
    // Remove all children from model root
    while (m_modelRoot->getNumChildren() > 0) {
        m_modelRoot->removeChild(0);
    }
}

void QuarterOcctViewer::setBackgroundColor(float r, float g, float b)
{
    m_quarterWidget->setBackgroundColor(QColor::fromRgbF(r, g, b));
}

void QuarterOcctViewer::setWireframe(bool wireframe)
{
    m_wireframeMode = wireframe;
    
    // Find draw style node and update it
    for (int i = 0; i < m_root->getNumChildren(); ++i) {
        SoNode* node = m_root->getChild(i);
        if (node->isOfType(SoDrawStyle::getClassTypeId())) {
            SoDrawStyle* drawStyle = static_cast<SoDrawStyle*>(node);
            drawStyle->style.setValue(wireframe ? SoDrawStyle::LINES : SoDrawStyle::FILLED);
            break;
        }
    }
}

void QuarterOcctViewer::zoomAll()
{
    if (m_modelRoot->getNumChildren() > 0) {
        // Calculate bounding box and adjust camera position
        m_quarterWidget->viewAll();
    }
}

void QuarterOcctViewer::onWireframeChanged(bool checked)
{
    setWireframe(checked);
}

void QuarterOcctViewer::onZoomChanged(int value)
{
    // Calculate zoom factor from slider value (1 to 100)
    float zoomFactor = static_cast<float>(value) / 50.0f;
    
    // Find camera and adjust position
    for (int i = 0; i < m_root->getNumChildren(); ++i) {
        SoNode* node = m_root->getChild(i);
        if (node->isOfType(SoPerspectiveCamera::getClassTypeId())) {
            SoPerspectiveCamera* camera = static_cast<SoPerspectiveCamera*>(node);
            SbVec3f position = camera->position.getValue();
            
            // Adjust camera distance based on zoom factor
            SbVec3f direction(0, 0, -1);
            SbVec3f newPosition = position * (zoomFactor / position.length());
            camera->position.setValue(newPosition);
            break;
        }
    }
}

void QuarterOcctViewer::onResetView()
{
    // Reset camera position
    for (int i = 0; i < m_root->getNumChildren(); ++i) {
        SoNode* node = m_root->getChild(i);
        if (node->isOfType(SoPerspectiveCamera::getClassTypeId())) {
            SoPerspectiveCamera* camera = static_cast<SoPerspectiveCamera*>(node);
            camera->position.setValue(0, 0, 10);
            camera->orientation.setValue(0, 0, 0, 1);
            break;
        }
    }
    
    // Reset zoom slider
    m_zoomSlider->setValue(50);
}

void QuarterOcctViewer::onAbout()
{
    QMessageBox::about(this, "About Quarter OCCT Viewer", 
        "Quarter OCCT Viewer v1.0\n" 
        "A Qt application for displaying OCCT models using Quarter (Coin3D)\n" 
        "\n" 
        "Features:\n" 
        "- Load and display STEP files\n" 
        "- Load and display IGES files\n" 
        "- Wireframe and solid rendering\n" 
        "- Zoom, pan, and rotate the model\n" 
        "- Mouse wheel zooming\n" 
        "- Interactive UI controls\n" 
        "\n" 
        "Uses:\n" 
        "- Qt Framework\n" 
        "- Coin3D\n" 
        "- Quarter\n" 
        "- Open CASCADE Technology (OCCT)");
}

void QuarterOcctViewer::wheelEvent(QWheelEvent* event)
{
    // Get the wheel delta (positive for forward, negative for backward)
    int delta = event->angleDelta().y();
    
    // Calculate zoom factor (1.1 for zoom in, 0.9 for zoom out)
    float zoomFactor = (delta > 0) ? 0.9f : 1.1f;
    
    // Get current camera position
    SbVec3f position = m_camera->position.getValue();
    
    // Calculate new position by moving along the view direction
    SbVec3f newPosition = position * zoomFactor;
    
    // Update camera position
    m_camera->position.setValue(newPosition);
    
    // Accept the event
    event->accept();
}
