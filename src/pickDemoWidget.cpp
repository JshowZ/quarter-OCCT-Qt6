#include "pickDemoWidget.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QMessageBox>

// Coin3D headers for highlighting
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/details/SoFaceDetail.h>
#include <Inventor/details/SoDetail.h>
#include <Inventor/elements/SoGLTextureEnabledElement.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/SbBox3f.h>

pickDemoWidget::pickDemoWidget(QWidget *parent)
    : QWidget(parent),
      m_quarterWidget(nullptr),
      m_root(nullptr),
      m_modelRoot(nullptr),
      m_camera(nullptr),
      m_pickRoot(nullptr),
      m_highlightRoot(nullptr),
      m_coords(nullptr),
      m_faceSet(nullptr),
      m_vertexHighlightMat(nullptr),
      m_edgeHighlightMat(nullptr),
      m_faceHighlightMat(nullptr),
      m_highlightedVertex(-1),
      m_highlightedEdge(-1),
      m_highlightedFace(-1)
{
    setupUI();
    SoInteraction::init();
}

pickDemoWidget::~pickDemoWidget()
{
    if (m_root) {
        m_root->unref();
    }
}

void pickDemoWidget::setupUI()
{
    // Create layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Create Quarter widget for 3D rendering
    m_quarterWidget = new SIM::Coin3D::Quarter::QuarterWidget(this);
    layout->addWidget(m_quarterWidget);

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
    
    // Add material for the cube
    SoMaterial* material = new SoMaterial;
    material->diffuseColor.setValue(0.8, 0.8, 0.8);
    m_root->addChild(material);
    
    // Add pick root separator for picking functionality
    m_pickRoot = new SoSeparator;
    m_root->addChild(m_pickRoot);
    
    // Add model root
    m_modelRoot = new SoSeparator;
    m_pickRoot->addChild(m_modelRoot);
    
    // Add highlight root
    m_highlightRoot = new SoSeparator;
    m_pickRoot->addChild(m_highlightRoot);
    
    // Create highlight materials
    m_vertexHighlightMat = new SoMaterial;
    m_vertexHighlightMat->diffuseColor.setValue(1.0, 0.0, 0.0); // Red for vertices
    
    m_edgeHighlightMat = new SoMaterial;
    m_edgeHighlightMat->diffuseColor.setValue(0.0, 1.0, 0.0); // Green for edges
    
    m_faceHighlightMat = new SoMaterial;
    m_faceHighlightMat->diffuseColor.setValue(0.0, 0.0, 1.0); // Blue for faces
    m_faceHighlightMat->transparency.setValue(0.5); // Semi-transparent
    
    // Create the cube
    createCube();
    
    // Set the scene graph to Quarter widget
    m_quarterWidget->setSceneGraph(m_root);
    
    // Set mouse tracking
    this->setMouseTracking(true);
    m_quarterWidget->setMouseTracking(true);
}

void pickDemoWidget::createCube()
{
    // Define cube vertices (8 vertices)
    m_cubeData.vertices[0].setValue(-1, -1, -1); // Back-bottom-left
    m_cubeData.vertices[1].setValue(1, -1, -1);  // Back-bottom-right
    m_cubeData.vertices[2].setValue(1, 1, -1);   // Back-top-right
    m_cubeData.vertices[3].setValue(-1, 1, -1);  // Back-top-left
    m_cubeData.vertices[4].setValue(-1, -1, 1);  // Front-bottom-left
    m_cubeData.vertices[5].setValue(1, -1, 1);   // Front-bottom-right
    m_cubeData.vertices[6].setValue(1, 1, 1);    // Front-top-right
    m_cubeData.vertices[7].setValue(-1, 1, 1);   // Front-top-left
    
    // Define cube edges (12 edges)
    m_cubeData.edges[0] = {0, 1};  // Back-bottom
    m_cubeData.edges[1] = {1, 2};  // Back-right
    m_cubeData.edges[2] = {2, 3};  // Back-top
    m_cubeData.edges[3] = {3, 0};  // Back-left
    m_cubeData.edges[4] = {4, 5};  // Front-bottom
    m_cubeData.edges[5] = {5, 6};  // Front-right
    m_cubeData.edges[6] = {6, 7};  // Front-top
    m_cubeData.edges[7] = {7, 4};  // Front-left
    m_cubeData.edges[8] = {0, 4};  // Left-bottom
    m_cubeData.edges[9] = {1, 5};  // Right-bottom
    m_cubeData.edges[10] = {2, 6}; // Right-top
    m_cubeData.edges[11] = {3, 7}; // Left-top
    
    // Define cube faces (6 faces)
    m_cubeData.faces[0] = {0, 1, 2, 3};  // Back face
    m_cubeData.faces[1] = {4, 5, 6, 7};  // Front face
    m_cubeData.faces[2] = {1, 5, 6, 2};  // Right face
    m_cubeData.faces[3] = {0, 4, 7, 3};  // Left face
    m_cubeData.faces[4] = {3, 2, 6, 7};  // Top face
    m_cubeData.faces[5] = {0, 1, 5, 4};  // Bottom face
    
    // Create coordinate node with 24 vertices (4 per face)
    m_coords = new SoCoordinate3;
    m_coords->point.setNum(24);
    
    // Back face (z=-1)
    m_coords->point.set1Value(0, m_cubeData.vertices[0]);
    m_coords->point.set1Value(1, m_cubeData.vertices[1]);
    m_coords->point.set1Value(2, m_cubeData.vertices[2]);
    m_coords->point.set1Value(3, m_cubeData.vertices[3]);
    
    // Front face (z=1)
    m_coords->point.set1Value(4, m_cubeData.vertices[4]);
    m_coords->point.set1Value(5, m_cubeData.vertices[5]);
    m_coords->point.set1Value(6, m_cubeData.vertices[6]);
    m_coords->point.set1Value(7, m_cubeData.vertices[7]);
    
    // Right face (x=1)
    m_coords->point.set1Value(8, m_cubeData.vertices[1]);
    m_coords->point.set1Value(9, m_cubeData.vertices[5]);
    m_coords->point.set1Value(10, m_cubeData.vertices[6]);
    m_coords->point.set1Value(11, m_cubeData.vertices[2]);
    
    // Left face (x=-1)
    m_coords->point.set1Value(12, m_cubeData.vertices[0]);
    m_coords->point.set1Value(13, m_cubeData.vertices[4]);
    m_coords->point.set1Value(14, m_cubeData.vertices[7]);
    m_coords->point.set1Value(15, m_cubeData.vertices[3]);
    
    // Top face (y=1)
    m_coords->point.set1Value(16, m_cubeData.vertices[3]);
    m_coords->point.set1Value(17, m_cubeData.vertices[2]);
    m_coords->point.set1Value(18, m_cubeData.vertices[6]);
    m_coords->point.set1Value(19, m_cubeData.vertices[7]);
    
    // Bottom face (y=-1)
    m_coords->point.set1Value(20, m_cubeData.vertices[0]);
    m_coords->point.set1Value(21, m_cubeData.vertices[1]);
    m_coords->point.set1Value(22, m_cubeData.vertices[5]);
    m_coords->point.set1Value(23, m_cubeData.vertices[4]);
    
    // Create face set
    m_faceSet = new SoFaceSet;
    m_faceSet->numVertices.setNum(6);
    for (int i = 0; i < 6; i++) {
        m_faceSet->numVertices.set1Value(i, 4);
    }
    
    // Create cube separator
    SoSeparator* cubeSep = new SoSeparator;
    cubeSep->addChild(m_coords);
    cubeSep->addChild(m_faceSet);
    
    m_modelRoot->addChild(cubeSep);
}

void pickDemoWidget::mouseMoveEvent(QMouseEvent* event)
{
    // Get mouse position in widget coordinates
    QPoint pos = event->pos();
    
    // Convert to viewport coordinates (Y is flipped in Coin3D)
    int x = pos.x();
    int y = m_quarterWidget->size().height() - pos.y();
    
    // Perform picking
    performPick(x, y);
    
    // Call the base class implementation
    QWidget::mouseMoveEvent(event);
}

void pickDemoWidget::performPick(int x, int y)
{
    // Reset highlighted elements
    m_highlightedVertex = -1;
    m_highlightedEdge = -1;
    m_highlightedFace = -1;
    
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
    
    // Get the picked point (if any)
    SoPickedPoint* pickedPoint = pickAction.getPickedPoint();
    if (pickedPoint) {
        // Get detail and check if it's a face detail
        const SoDetail* detail = pickedPoint->getDetail();
        if (detail && detail->getTypeId().isDerivedFrom(SoFaceDetail::getClassTypeId())) {
            // Cast to SoFaceDetail
            const SoFaceDetail* faceDetail = static_cast<const SoFaceDetail*>(detail);
            
            // Get face index
            int faceIndex = faceDetail->getFaceIndex();
            if (faceIndex >= 0 && faceIndex < 6) {
                m_highlightedFace = faceIndex;
            }
            
            // Get number of vertices in the face
            int numVertices = faceDetail->getNumVertices();
            if (numVertices > 0) {
                // Check distance to each vertex of the face
                SbVec3f intersection = pickedPoint->getPoint();
                float minDist = 0.3f; // Threshold for vertex highlighting
                
                // Create array to store vertex indices
                int vertexIndices[4] = { -1, -1, -1, -1 };
                
                // Get vertex indices one by one
                for (int i = 0; i < numVertices && i < 4; i++) {
                    vertexIndices[i] = faceDetail->getVertexIndex(i);
                }
                
                for (int i = 0; i < 4; i++) {
                    if (vertexIndices[i] == -1) break;
                    
                    int vertexIndex = vertexIndices[i];
                    // Map from 24-vertex index to 8-vertex index
                    int cubeVertexIndex = -1;
                    for (int j = 0; j < 8; j++) {
                        if (m_cubeData.vertices[j] == m_coords->point[vertexIndex]) {
                            cubeVertexIndex = j;
                            break;
                        }
                    }
                    
                    if (cubeVertexIndex != -1) {
                        float dist = (intersection - m_cubeData.vertices[cubeVertexIndex]).length();
                        if (dist < minDist) {
                            minDist = dist;
                            m_highlightedVertex = cubeVertexIndex;
                            m_highlightedFace = -1; // Prioritize vertex over face
                        }
                    }
                }
                
                // Check edges if no vertex is highlighted
                if (m_highlightedVertex == -1) {
                    float edgeThreshold = 0.2f;
                    
                    // Check each edge of the face
                    for (int i = 0; i < 4; i++) {
                        if (vertexIndices[i] == -1 || vertexIndices[(i + 1) % 4] == -1) break;
                        
                        int v1 = vertexIndices[i];
                        int v2 = vertexIndices[(i + 1) % 4];
                        
                        // Get the two vertices of the edge
                        SbVec3f p1 = m_coords->point[v1];
                        SbVec3f p2 = m_coords->point[v2];
                        
                        // Calculate distance from intersection to edge
                        SbVec3f edge = p2 - p1;
                        SbVec3f vp = intersection - p1;
                        float t = edge.dot(vp) / edge.dot(edge);
                        // Clamp t between 0.0 and 1.0
                        if (t < 0.0f) t = 0.0f;
                        if (t > 1.0f) t = 1.0f;
                        SbVec3f closest = p1 + t * edge;
                        float dist = (intersection - closest).length();
                        
                        if (dist < edgeThreshold) {
                            // Find the edge index in the cube data
                            for (int j = 0; j < 12; j++) {
                                int cubeV1 = -1, cubeV2 = -1;
                                for (int k = 0; k < 8; k++) {
                                    if (m_cubeData.vertices[k] == p1) cubeV1 = k;
                                    if (m_cubeData.vertices[k] == p2) cubeV2 = k;
                                }
                                
                                if ((m_cubeData.edges[j].v1 == cubeV1 && m_cubeData.edges[j].v2 == cubeV2) ||
                                    (m_cubeData.edges[j].v1 == cubeV2 && m_cubeData.edges[j].v2 == cubeV1)) {
                                    m_highlightedEdge = j;
                                    m_highlightedFace = -1; // Prioritize edge over face
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
        }
        }
    }
    
    // Update highlight
    updateHighlight();
}

void pickDemoWidget::updateHighlight()
{
    // Clear previous highlights
    while (m_highlightRoot->getNumChildren() > 0) {
        m_highlightRoot->removeChild(0);
    }
    
    // Highlight vertex if selected
    if (m_highlightedVertex != -1) {
        SoSeparator* vertexSep = new SoSeparator;
        
        // Create draw style for vertex highlighting (large points)
        SoDrawStyle* drawStyle = new SoDrawStyle;
        drawStyle->pointSize.setValue(15.0f);
        vertexSep->addChild(drawStyle);
        
        // Add highlight material
        vertexSep->addChild(m_vertexHighlightMat);
        
        // Create point set for the highlighted vertex
        SoCoordinate3* vertexCoords = new SoCoordinate3;
        vertexCoords->point.set1Value(0, m_cubeData.vertices[m_highlightedVertex]);
        vertexSep->addChild(vertexCoords);
        
        SoPointSet* pointSet = new SoPointSet;
        pointSet->numPoints.setValue(1);
        vertexSep->addChild(pointSet);
        
        m_highlightRoot->addChild(vertexSep);
    }
    // Highlight edge if selected
    else if (m_highlightedEdge != -1) {
        SoSeparator* edgeSep = new SoSeparator;
        
        // Create draw style for edge highlighting (thick lines)
        SoDrawStyle* drawStyle = new SoDrawStyle;
        drawStyle->lineWidth.setValue(5.0f);
        edgeSep->addChild(drawStyle);
        
        // Add highlight material
        edgeSep->addChild(m_edgeHighlightMat);
        
        // Create coordinate node for the edge vertices
        SoCoordinate3* edgeCoords = new SoCoordinate3;
        edgeCoords->point.setNum(2);
        edgeCoords->point.set1Value(0, m_cubeData.vertices[m_cubeData.edges[m_highlightedEdge].v1]);
        edgeCoords->point.set1Value(1, m_cubeData.vertices[m_cubeData.edges[m_highlightedEdge].v2]);
        edgeSep->addChild(edgeCoords);
        
        // Create line set for the edge
        SoLineSet* lineSet = new SoLineSet;
        lineSet->numVertices.setValue(2);
        edgeSep->addChild(lineSet);
        
        m_highlightRoot->addChild(edgeSep);
    }
    // Highlight face if selected
    else if (m_highlightedFace != -1) {
        SoSeparator* faceSep = new SoSeparator;
        
        // Add highlight material
        faceSep->addChild(m_faceHighlightMat);
        
        // Create coordinate node for the face vertices
        SoCoordinate3* faceCoords = new SoCoordinate3;
        faceCoords->point.setNum(4);
        
        // Get the four vertices of the highlighted face
        int v1 = m_cubeData.faces[m_highlightedFace].v1;
        int v2 = m_cubeData.faces[m_highlightedFace].v2;
        int v3 = m_cubeData.faces[m_highlightedFace].v3;
        int v4 = m_cubeData.faces[m_highlightedFace].v4;
        
        faceCoords->point.set1Value(0, m_cubeData.vertices[v1]);
        faceCoords->point.set1Value(1, m_cubeData.vertices[v2]);
        faceCoords->point.set1Value(2, m_cubeData.vertices[v3]);
        faceCoords->point.set1Value(3, m_cubeData.vertices[v4]);
        faceSep->addChild(faceCoords);
        
        // Create face set for the highlighted face
        SoFaceSet* highlightFaceSet = new SoFaceSet;
        highlightFaceSet->numVertices.set1Value(0, 4);
        faceSep->addChild(highlightFaceSet);
        
        m_highlightRoot->addChild(faceSep);
    }
}
