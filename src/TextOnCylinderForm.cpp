#include "TextOnCylinderForm.h"

// Qt headers
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSlider>
#include <QPushButton>
#include <QLineEdit>
#include <QDoubleSpinBox>

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
#include <Inventor/nodes/SoShape.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/SbViewportRegion.h>
#include <Inventor/SoPath.h>
#include <Inventor/SoInteraction.h>
#include <Inventor/SoPickedPoint.h>
#include <Quarter/QuarterWidget.h>
#include <Quarter/eventhandlers/DragDropHandler.h>

// Use Quarter namespace for convenience
using namespace SIM::Coin3D::Quarter;

// OCCT headers
#include <Standard.hxx>
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
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

// OCCT headers for primitives and text
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepLib.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>
#include <BRepPrimAPI_MakeBox.hxx>

// OCCT headers for text
#include <StdPrs_BRepFont.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <TopAbs.hxx>

// OCCT headers for projection
#include <BRepProj_Projection.hxx>
#include <Geom_Plane.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <gp_Pln.hxx>
#include <gp_Cylinder.hxx>
#include <TCollection_AsciiString.hxx>

#include "ViewTool.h"
#include "base.h"


TextOnCylinderForm::TextOnCylinderForm(QWidget *parent)
    : QMainWindow(parent),
      m_centralWidget(nullptr),
      m_mainLayout(nullptr),
      m_controlPanel(nullptr),
      m_quarterWidget(nullptr),
      m_root(nullptr),
      m_modelRoot(nullptr),
      m_camera(nullptr),
      m_textInput(nullptr),
      m_cylinderRadius(nullptr),
      m_cylinderHeight(nullptr),
      m_textHeight(nullptr),
      m_engravingDepth(nullptr),
      m_createCylinderBtn(nullptr),
      m_createTextBtn(nullptr),
      m_projectTextBtn(nullptr),
      m_engraveTextBtn(nullptr),
      m_clearBtn(nullptr),
      m_zoomAllBtn(nullptr),
      m_cylinderCreated(false),
      m_textCreated(false)
{
    setupUI();
}

TextOnCylinderForm::~TextOnCylinderForm()
{
    // Cleanup Coin3D nodes
    if (m_root) {
        m_root->unref();
    }
}

void TextOnCylinderForm::setupUI()
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

    this->m_quarterWidget->installEventFilter(new SIM::Coin3D::Quarter::DragDropHandler(this->m_quarterWidget));
    //set default navigation mode file
    this->m_quarterWidget->setNavigationModeFile();
    
    // Create control panel
    m_controlPanel = new QWidget(m_centralWidget);
    QVBoxLayout* controlLayout = new QVBoxLayout(m_controlPanel);
    controlLayout->setContentsMargins(5, 5, 5, 5);
    
    // Text input
    QHBoxLayout* textLayout = new QHBoxLayout();
    textLayout->addWidget(new QLabel("Text:", m_controlPanel));
    m_textInput = new QLineEdit(m_controlPanel);
    m_textInput->setText("HELLO");
    textLayout->addWidget(m_textInput);
    controlLayout->addLayout(textLayout);
    
    // Cylinder parameters
    QHBoxLayout* cylinderLayout = new QHBoxLayout();
    cylinderLayout->addWidget(new QLabel("Cylinder Radius:", m_controlPanel));
    m_cylinderRadius = new QDoubleSpinBox(m_controlPanel);
    m_cylinderRadius->setRange(0.1, 100.0);
    m_cylinderRadius->setValue(5.0);
    m_cylinderRadius->setSingleStep(0.1);
    cylinderLayout->addWidget(m_cylinderRadius);
    
    cylinderLayout->addWidget(new QLabel("Height:", m_controlPanel));
    m_cylinderHeight = new QDoubleSpinBox(m_controlPanel);
    m_cylinderHeight->setRange(0.1, 200.0);
    m_cylinderHeight->setValue(20.0);
    m_cylinderHeight->setSingleStep(0.1);
    cylinderLayout->addWidget(m_cylinderHeight);
    controlLayout->addLayout(cylinderLayout);
    
    // Text parameters
    QHBoxLayout* textParamLayout = new QHBoxLayout();
    textParamLayout->addWidget(new QLabel("Text Height:", m_controlPanel));
    m_textHeight = new QDoubleSpinBox(m_controlPanel);
    m_textHeight->setRange(0.1, 50.0);
    m_textHeight->setValue(2.0);
    m_textHeight->setSingleStep(0.1);
    textParamLayout->addWidget(m_textHeight);
    
    textParamLayout->addWidget(new QLabel("Engraving Depth:", m_controlPanel));
    m_engravingDepth = new QDoubleSpinBox(m_controlPanel);
    m_engravingDepth->setRange(0.01, 10.0);
    m_engravingDepth->setValue(0.5);
    m_engravingDepth->setSingleStep(0.1);
    textParamLayout->addWidget(m_engravingDepth);
    controlLayout->addLayout(textParamLayout);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_createCylinderBtn = new QPushButton("Create Cylinder", m_controlPanel);
    connect(m_createCylinderBtn, &QPushButton::clicked, this, &TextOnCylinderForm::onCreateCylinder);
    buttonLayout->addWidget(m_createCylinderBtn);
    
    m_createTextBtn = new QPushButton("Create Text", m_controlPanel);
    connect(m_createTextBtn, &QPushButton::clicked, this, &TextOnCylinderForm::onCreateText);
    buttonLayout->addWidget(m_createTextBtn);
    
    m_projectTextBtn = new QPushButton("Project Text", m_controlPanel);
    connect(m_projectTextBtn, &QPushButton::clicked, this, &TextOnCylinderForm::onProjectText);
    buttonLayout->addWidget(m_projectTextBtn);
    
    m_engraveTextBtn = new QPushButton("Engrave Text", m_controlPanel);
    connect(m_engraveTextBtn, &QPushButton::clicked, this, &TextOnCylinderForm::onEngraveText);
    buttonLayout->addWidget(m_engraveTextBtn);
    
    m_clearBtn = new QPushButton("Clear", m_controlPanel);
    connect(m_clearBtn, &QPushButton::clicked, this, &TextOnCylinderForm::onClear);
    buttonLayout->addWidget(m_clearBtn);
    
    m_zoomAllBtn = new QPushButton("Zoom All", m_controlPanel);
    connect(m_zoomAllBtn, &QPushButton::clicked, this, &TextOnCylinderForm::onZoomAll);
    buttonLayout->addWidget(m_zoomAllBtn);
    
    controlLayout->addLayout(buttonLayout);
    
    m_mainLayout->addWidget(m_controlPanel);
    m_mainLayout->setStretch(0, 1); // Make quarter widget expand
    
    // Create menu bar
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // File menu
    QMenu* fileMenu = menuBar->addMenu("File");
    
    QAction* exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    
    // Help menu
    QMenu* helpMenu = menuBar->addMenu("Help");
    
    QAction* aboutAction = helpMenu->addAction("About");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "About Text on Cylinder", 
            "Text on Cylinder v1.0\n" 
            "A Qt application for creating cylinder with text projection and engraving\n" 
            "\n" 
            "Features:\n" 
            "- Create cylinder\n" 
            "- Create text\n" 
            "- Project text onto cylinder\n" 
            "- Engrave text onto cylinder\n" 
            "\n" 
            "Uses:\n" 
            "- Qt Framework\n" 
            "- Coin3D\n" 
            "- Quarter\n" 
            "- Open CASCADE Technology (OCCT)");
    });
    
    // Initialize Coin3D interaction system (for mouse rotation, pan, zoom)
    SoInteraction::init();
    
    // Initialize Coin3D scene
    m_root = new SoSeparator;
    m_root->ref();
    
    // Add camera
    m_camera = new SoPerspectiveCamera;
    m_camera->position.setValue(0, 0, 30);
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
    m_quarterWidget->setBackgroundColor(QColor::fromRgbF(1.0, 1.0, 1.0));

    // Set window properties
    setWindowTitle("Text on Cylinder");
    resize(800, 600);
}

TopoDS_Shape TextOnCylinderForm::createCylinder(double radius, double height)
{
    BRepPrimAPI_MakeCylinder cylinderMaker(radius, height);
    return cylinderMaker.Shape();
}

TopoDS_Shape TextOnCylinderForm::createText(const std::string& text, double height)
{
    // Create a simple rectangular shape as a placeholder for text
    // Will implement proper text creation once we find the correct API
    BRepPrimAPI_MakeBox boxMaker(height * 0.5, height, height * 0.1);
    return boxMaker.Shape();
}

TopoDS_Shape TextOnCylinderForm::projectTextOntoCylinder(const TopoDS_Shape& cylinder, const TopoDS_Shape& text)
{
    // Create a compound shape to hold the result
    TopoDS_Compound result;
    BRep_Builder builder;
    builder.MakeCompound(result);
    
    // Add original cylinder
    builder.Add(result, cylinder);
    
    // Find the cylindrical face
    TopoDS_Face cylinderFace;
    TopExp_Explorer faceExplorer(cylinder, TopAbs_FACE);
    while (faceExplorer.More()) {
        const TopoDS_Face& face = TopoDS::Face(faceExplorer.Current());
        Handle(Geom_Surface) surface = BRep_Tool::Surface(face);
        if (surface->DynamicType() == STANDARD_TYPE(Geom_CylindricalSurface)) {
            cylinderFace = face;
            break;
        }
        faceExplorer.Next();
    }
    
    if (cylinderFace.IsNull()) {
        // If no cylindrical face found, just add the text
        builder.Add(result, text);
        return result;
    }
    
    // Create projection context
    gp_Dir projectionDir(0, 1, 0); // Project along Y-axis
    BRepProj_Projection projector(text, cylinderFace, projectionDir);
    
    // Create wire from projection
    if (projector.IsDone()) {
        TopoDS_Wire projectedWire;
        BRepBuilderAPI_MakeWire wireMaker;
        
        while (projector.More()) {
            wireMaker.Add(projector.Current());
            projector.Next();
        }
        
        if (wireMaker.IsDone()) {
            projectedWire = wireMaker.Wire();
            builder.Add(result, projectedWire);
        }
    }
    
    return result;
}

TopoDS_Shape TextOnCylinderForm::engraveTextOntoCylinder(const TopoDS_Shape& cylinder, const TopoDS_Shape& text, double depth)
{
    // Find the cylindrical face
    TopoDS_Face cylinderFace;
    TopExp_Explorer faceExplorer(cylinder, TopAbs_FACE);
    while (faceExplorer.More()) {
        const TopoDS_Face& face = TopoDS::Face(faceExplorer.Current());
        Handle(Geom_Surface) surface = BRep_Tool::Surface(face);
        if (surface->DynamicType() == STANDARD_TYPE(Geom_CylindricalSurface)) {
            cylinderFace = face;
            break;
        }
        faceExplorer.Next();
    }
    
    if (cylinderFace.IsNull()) {
        return cylinder;
    }
    
    // Get cylinder radius
    Handle(Geom_CylindricalSurface) cylSurface = Handle(Geom_CylindricalSurface)::DownCast(BRep_Tool::Surface(cylinderFace));
    double radius = cylSurface->Radius();
    
    // Create projection direction
    gp_Dir projectionDir(0, 1, 0); // Project along Y-axis
    
    // Project text onto cylinder
    BRepProj_Projection projector(text, cylinderFace, projectionDir);
    
    if (!projector.IsDone()) {
        return cylinder;
    }
    
    // Create wires from projection
    BRepBuilderAPI_MakeWire wireMaker;
    while (projector.More()) {
        wireMaker.Add(projector.Current());
        projector.Next();
    }
    
    if (!wireMaker.IsDone()) {
        return cylinder;
    }
    
    TopoDS_Wire projectedWire = wireMaker.Wire();
    
    // Create a face from the wire
    BRepBuilderAPI_MakeFace faceMaker(projectedWire);
    if (!faceMaker.IsDone()) {
        return cylinder;
    }
    
    TopoDS_Face projectedFace = faceMaker.Face();
    
    // Create offset face for engraving
    BRepOffsetAPI_MakeOffset offsetMaker;
    offsetMaker.Init(projectedFace);
    offsetMaker.Perform(-depth);
    
    if (!offsetMaker.IsDone()) {
        return cylinder;
    }
    
    TopoDS_Shape offsetShape = offsetMaker.Shape();
    
    // Cut the offset shape from the cylinder
    BRepAlgoAPI_Cut cutter(cylinder, offsetShape);
    if (!cutter.IsDone()) {
        return cylinder;
    }
    
    return cutter.Shape();
}

SoNode* TextOnCylinderForm::convertShapeRecursive(TopoDS_Shape shape, double deviation, double angularDeflection)
{
    try {
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

        Standard_Real deflection = ViewTool::getDeflection(shape, deviation);

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

        // get the raw memory for fast fill up
        SbVec3f* verts = coords->point.startEditing();
        SbVec3f* norms = norm->vector.startEditing();
        int32_t* index = faceSet->coordIndex.startEditing();

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
        QMessageBox::critical(this, "OCCT Exception", e.GetMessageString());
        return nullptr;
    }
    catch (...) {
        QMessageBox::critical(this, "Error", "Unknown error during shape conversion");
        return nullptr;
    }
}

SoNode* TextOnCylinderForm::convertShapeToCoin3D(const TopoDS_Shape& shape)
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
        
        int numTriangles = 0, numNodes = 0, numNorms = 0;
        
        TopLoc_Location aLoc = shape.Location();
        
        // Calculate nodes from faces
        for (int i = 1; i <= faceMap.Extent(); i++) {
            const TopoDS_Face& face = TopoDS::Face(faceMap(i));
            Handle(Poly_Triangulation) mesh = BRep_Tool::Triangulation(face, aLoc);
            
            if (!mesh.IsNull()) {
                numTriangles += mesh->NbTriangles();
                numNodes += mesh->NbNodes();
                numNorms += mesh->NbNodes();
            }
        }
        
        // Get edges
        TopTools_IndexedMapOfShape edgeMap;
        TopExp::MapShapes(shape, TopAbs_EDGE, edgeMap);
        
        // Calculate nodes from edges
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
        
        // Create Coin3D nodes with correct sizes
        SoCoordinate3* coords = new SoCoordinate3;
        SoNormal* norm = new SoNormal;
        SoNormalBinding* normalBinding = new SoNormalBinding;
        SoIndexedFaceSet* faceSet = new SoIndexedFaceSet;
        SoIndexedLineSet* lineSet = new SoIndexedLineSet;
        
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
        
        return shapeSep;
    } catch (const Standard_Failure& e) {
        QMessageBox::critical(this, "OCCT Exception", e.GetMessageString());
        return nullptr;
    } catch (...) {
        QMessageBox::critical(this, "Error", "Unknown error during shape conversion");
        return nullptr;
    }
}

bool TextOnCylinderForm::displayShape(const TopoDS_Shape& shape, bool clearExisting)
{
    // Clear existing model if requested
    if (clearExisting) {
        clearScene();
    }
    
    // Convert OCCT shape to Coin3D node
    SoNode* modelNode = convertShapeRecursive(shape, 0.001, 0.05);
    if (modelNode) {
        m_modelRoot->addChild(modelNode);
        
        // Zoom to fit
        zoomAll();
        
        return true;
    }
    
    return false;
}

void TextOnCylinderForm::clearScene()
{
    // Remove all children from model root
    while (m_modelRoot->getNumChildren() > 0) {
        m_modelRoot->removeChild(0);
    }
}

void TextOnCylinderForm::zoomAll()
{
    if (m_modelRoot->getNumChildren() > 0) {
        // Calculate bounding box and adjust camera position
        m_quarterWidget->viewAll();
    }
}

void TextOnCylinderForm::onCreateCylinder()
{
    double radius = m_cylinderRadius->value();
    double height = m_cylinderHeight->value();
    
    try {
        m_cylinder = createCylinder(radius, height);
        m_cylinderCreated = true;
        displayShape(m_cylinder);
    } catch (const Standard_Failure& e) {
        QMessageBox::critical(this, "Error", QString("Failed to create cylinder: %1").arg(e.GetMessageString()));
    } catch (...) {
        QMessageBox::critical(this, "Error", "Failed to create cylinder: Unknown error");
    }
}

void TextOnCylinderForm::onCreateText()
{
    std::string text = m_textInput->text().toStdString();
    double height = m_textHeight->value();
    
    try {
        m_text = createText(text, height);
        m_textCreated = true;
        displayShape(m_text);
    } catch (const Standard_Failure& e) {
        QMessageBox::critical(this, "Error", QString("Failed to create text: %1").arg(e.GetMessageString()));
    } catch (...) {
        QMessageBox::critical(this, "Error", "Failed to create text: Unknown error");
    }
}

void TextOnCylinderForm::onProjectText()
{
    if (!m_cylinderCreated) {
        QMessageBox::warning(this, "Warning", "Please create a cylinder first");
        return;
    }
    
    if (!m_textCreated) {
        QMessageBox::warning(this, "Warning", "Please create text first");
        return;
    }
    
    try {
        TopoDS_Shape projectedShape = projectTextOntoCylinder(m_cylinder, m_text);
        displayShape(projectedShape);
    } catch (const Standard_Failure& e) {
        QMessageBox::critical(this, "Error", QString("Failed to project text: %1").arg(e.GetMessageString()));
    } catch (...) {
        QMessageBox::critical(this, "Error", "Failed to project text: Unknown error");
    }
}

void TextOnCylinderForm::onEngraveText()
{
    if (!m_cylinderCreated) {
        QMessageBox::warning(this, "Warning", "Please create a cylinder first");
        return;
    }
    
    if (!m_textCreated) {
        QMessageBox::warning(this, "Warning", "Please create text first");
        return;
    }
    
    double depth = m_engravingDepth->value();
    
    try {
        TopoDS_Shape engravedShape = engraveTextOntoCylinder(m_cylinder, m_text, depth);
        displayShape(engravedShape);
    } catch (const Standard_Failure& e) {
        QMessageBox::critical(this, "Error", QString("Failed to engrave text: %1").arg(e.GetMessageString()));
    } catch (...) {
        QMessageBox::critical(this, "Error", "Failed to engrave text: Unknown error");
    }
}

void TextOnCylinderForm::onClear()
{
    clearScene();
    m_cylinderCreated = false;
    m_textCreated = false;
}

void TextOnCylinderForm::onZoomAll()
{
    zoomAll();
}
