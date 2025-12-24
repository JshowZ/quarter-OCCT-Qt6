#include "CubeWindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QMessageBox>

// Coin3D headers
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoFaceSet.h>
#include <Quarter/QuarterWidget.h>

CubeWindow::CubeWindow(QWidget *parent)
    : QMainWindow(parent),
      m_centralWidget(nullptr),
      m_mainLayout(nullptr),
      m_quarterWidget(nullptr),
      m_root(nullptr),
      m_modelRoot(nullptr)
{
    setupUI();
}

CubeWindow::~CubeWindow()
{
    if (m_root) {
        m_root->unref();
    }
}

void CubeWindow::setupUI()
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
    
    // Initialize Coin3D scene
    m_root = new SoSeparator;
    m_root->ref();
    
    // Add camera
    SoPerspectiveCamera* camera = new SoPerspectiveCamera;
    camera->position.setValue(0, 0, 10);
    camera->heightAngle.setValue(M_PI / 4.0);
    camera->nearDistance.setValue(0.1);
    camera->farDistance.setValue(1000.0);
    m_root->addChild(camera);
    
    // Add light
    SoDirectionalLight* light = new SoDirectionalLight;
    light->direction.setValue(-0.5, -0.5, -1);
    m_root->addChild(light);
    
    // Add shape hints for better rendering
    SoShapeHints* shapeHints = new SoShapeHints;
    shapeHints->vertexOrdering.setValue(SoShapeHints::COUNTERCLOCKWISE);
    m_root->addChild(shapeHints);
    
    // Add material
    SoMaterial* material = new SoMaterial;
    material->diffuseColor.setValue(0.8, 0.8, 0.8);
    m_root->addChild(material);
    
    // Add model root
    m_modelRoot = new SoSeparator;
    m_root->addChild(m_modelRoot);
    
    // Create the cube
    createCube();
    
    // Set the scene graph to Quarter widget
    m_quarterWidget->setSceneGraph(m_root);
    
    // Set window properties
    setWindowTitle("Cube Viewer");
    resize(800, 600);
}

void CubeWindow::createCube()
{
    SoFaceSet* faceSet = new SoFaceSet;
    faceSet->numVertices.setNum(6);
    faceSet->numVertices.set1Value(0, 4); // Back face
    faceSet->numVertices.set1Value(1, 4); // Front face
    faceSet->numVertices.set1Value(2, 4); // Right face
    faceSet->numVertices.set1Value(3, 4); // Left face
    faceSet->numVertices.set1Value(4, 4); // Top face
    faceSet->numVertices.set1Value(5, 4); // Bottom face

    SoCoordinate3* coords = new SoCoordinate3;
    coords->point.setNum(24);
    // Back face (z=-1)
    coords->point.set1Value(0, -1, -1, -1);
    coords->point.set1Value(1,  1, -1, -1);
    coords->point.set1Value(2,  1,  1, -1);
    coords->point.set1Value(3, -1,  1, -1);
    // Front face (z=1)
    coords->point.set1Value(4, -1, -1,  1);
    coords->point.set1Value(5,  1, -1,  1);
    coords->point.set1Value(6,  1,  1,  1);
    coords->point.set1Value(7, -1,  1,  1);
    // Right face (x=1)
    coords->point.set1Value(8,  1, -1, -1);
    coords->point.set1Value(9,  1, -1,  1);
    coords->point.set1Value(10, 1,  1,  1);
    coords->point.set1Value(11, 1,  1, -1);
    // Left face (x=-1)
    coords->point.set1Value(12, -1, -1, -1);
    coords->point.set1Value(13, -1,  1, -1);
    coords->point.set1Value(14, -1,  1,  1);
    coords->point.set1Value(15, -1, -1,  1);
    // Top face (y=1)
    coords->point.set1Value(16, -1,  1, -1);
    coords->point.set1Value(17,  1,  1, -1);
    coords->point.set1Value(18,  1,  1,  1);
    coords->point.set1Value(19, -1,  1,  1);
    // Bottom face (y=-1)
    coords->point.set1Value(20, -1, -1, -1);
    coords->point.set1Value(21, -1, -1,  1);
    coords->point.set1Value(22,  1, -1,  1);
    coords->point.set1Value(23,  1, -1, -1);

    SoSeparator* cubeSep = new SoSeparator;
    cubeSep->addChild(coords);
    cubeSep->addChild(faceSet);
    
    m_modelRoot->addChild(cubeSep);
}