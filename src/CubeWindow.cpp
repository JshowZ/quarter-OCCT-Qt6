#include "CubeWindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QMouseEvent>

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
#include <Inventor/nodes/SoShape.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/SbViewportRegion.h>
#include <Inventor/SoPath.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/SoInteraction.h>

#include <Quarter/eventhandlers/DragDropHandler.h>
#include <Quarter/QuarterWidget.h>

CubeWindow::CubeWindow(QWidget *parent)
    : QMainWindow(parent),
      m_quarterWidget(nullptr),
      m_root(nullptr),
      m_modelRoot(nullptr),
      m_camera(nullptr),
      m_pickRoot(nullptr),
      m_pickEnabled(true)
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

    // Create Quarter widget for 3D rendering
    m_quarterWidget = new SIM::Coin3D::Quarter::QuarterWidget(this);
    setCentralWidget(m_quarterWidget);

    this->m_quarterWidget->installEventFilter(new SIM::Coin3D::Quarter::DragDropHandler(this->m_quarterWidget));
    //set default navigation mode file
    this->m_quarterWidget->setNavigationModeFile();
    
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
    
    // Add material
    SoMaterial* material = new SoMaterial;
    material->diffuseColor.setValue(0.8, 0.8, 0.8);
    m_root->addChild(material);
    
    // Add pick root separator for picking functionality
    m_pickRoot = new SoSeparator;
    m_root->addChild(m_pickRoot);
    
    // Add model root
    m_modelRoot = new SoSeparator;
    m_pickRoot->addChild(m_modelRoot);
    
    // Create the cube
    createCube();
    
    // Set the scene graph to Quarter widget
    m_quarterWidget->setSceneGraph(m_root);
    
    // Set window properties
    setWindowTitle("Cube Viewer");
    resize(800, 600);

    this->setMouseTracking(true);
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

void CubeWindow::mousePressEvent(QMouseEvent* event)
{
    // If left mouse button is pressed, perform picking
    if (event->button() == Qt::LeftButton && m_pickEnabled)
    {
        // Get mouse position in widget coordinates
        QPoint pos = event->pos();
        
        // Convert to viewport coordinates (Y is flipped in Coin3D)
        int x = pos.x();
        int y = m_quarterWidget->size().height() - pos.y();
        
        // Perform picking
        performPick(x, y);
    }
    
    // Call the base class implementation for other mouse events
    QMainWindow::mousePressEvent(event);
}

void CubeWindow::performPick(int x, int y)
{
    // Create viewport region from widget size
    SbViewportRegion viewport(m_quarterWidget->size().width(), m_quarterWidget->size().height());
    
    // Create ray pick action for the current viewport
    SoRayPickAction pickAction(viewport);
    pickAction.setPoint(SbVec2s(x, y));
    
    // Set up pick action parameters for better picking accuracy
    pickAction.setRadius(5.0f); // Set pick radius for easier picking
    pickAction.setPickAll(false); // Only pick the nearest object
    
    // Apply the pick action to the scene graph
    pickAction.apply(m_pickRoot);
    
    // Get the picked path (if any)
    SoPickedPoint* pickedPoint = pickAction.getPickedPoint();
    if (pickedPoint)
    {
        SoPath* pickedPath = pickedPoint->getPath();
        if (pickedPath)
        {
            // Get the picked shape node
            SoNode* pickedNode = pickedPath->getTail();
            if (pickedNode && pickedNode->isOfType(SoShape::getClassTypeId()))
            {
                // Display information about the picked object
                QString message = QString("Picked shape: %1\n").arg(pickedNode->getTypeId().getName().getString());
                
                // Get the intersection point in world coordinates
                SbVec3f intersection = pickedPoint->getPoint();
                message += QString("Intersection point: (%.3f, %.3f, %.3f)\n").arg(intersection[0]).arg(intersection[1]).arg(intersection[2]);
                
                // Get the normal at the intersection point
                SbVec3f normal = pickedPoint->getNormal();
                message += QString("Normal vector: (%.3f, %.3f, %.3f)").arg(normal[0]).arg(normal[1]).arg(normal[2]);
                
                // Show the picked information
                QMessageBox::information(this, "Object Picked", message);
            }
        }
    }
}