#include "quarterviewer.h"
#include <QVBoxLayout>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/manips/SoTrackballManip.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoCone.h>
#include <QMessageBox>

// OCCT related headers
#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <WNT_Window.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <StdPrs_Point.hxx>
#include <StdPrs_WFShape.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_LineAspect.hxx>
#include <BRep_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS.hxx>
#include <gp_Pnt.hxx>

QuarterViewer::QuarterViewer(QWidget *parent) : QWidget(parent)
{
    // Set window attributes to ensure correct OpenGL rendering
    this->setAttribute(Qt::WA_PaintOnScreen);
    this->setAttribute(Qt::WA_OpaquePaintEvent);
    this->setAttribute(Qt::WA_NoSystemBackground);
    this->setFocusPolicy(Qt::StrongFocus);
    
    // Create layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // Initialize Quarter
    Quarter::init();
    
    // Create QuarterWidget and set key attributes
    m_quarterWidget = new QuarterWidget(this);
    m_quarterWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_quarterWidget->setFocusPolicy(Qt::StrongFocus);
    layout->addWidget(m_quarterWidget);
    
    qDebug() << "QuarterViewer: Widget created with OpenGL attributes";
    qDebug() << "QuarterViewer: Quarter initialized";
    qDebug() << "QuarterViewer: QuarterWidget created";
    
    // Initialize other members
    m_rootNode = new SoSeparator();
    m_rootNode->ref();
    
    m_shapesRoot = new SoSeparator();
    m_shapesRoot->ref();

    SoBaseColor* col = new SoBaseColor;
    col->rgb = SbColor(1, 1, 0);
    m_rootNode->addChild(col);

    m_rootNode->addChild(new SoCone);
    //m_rootNode->addChild(m_shapesRoot);
    //
    //// Create camera
    //SoPerspectiveCamera *camera = new SoPerspectiveCamera();
    //camera->position.setValue(0, 0, 200); // Increase camera distance, expand field of view
    //camera->orientation.setValue(0, 0, 0, 1);
    //camera->heightAngle = M_PI / 3; // Increase viewing angle, see more content
    //camera->nearDistance = 1;
    //camera->farDistance = 1000;
    //m_rootNode->addChild(camera);
    //
    //// Add light
    //SoDirectionalLight *light = new SoDirectionalLight();
    //light->direction.setValue(-1, -1, -1);
    //light->intensity.setValue(0.8);
    //m_rootNode->addChild(light);
    //
    //// Add interaction controls, allow user to rotate, scale and pan view
    //SoTrackballManip *manipulator = new SoTrackballManip();
    //m_rootNode->addChild(manipulator);
    //
    //qDebug() << "QuarterViewer: Camera position:" << camera->position.getValue()[0] << "," << camera->position.getValue()[1] << "," << camera->position.getValue()[2];
    //
    //// Set scene graph
    //m_quarterWidget->setSceneGraph(m_rootNode);
    //
    //m_currentMode = 0;
}

QuarterViewer::~QuarterViewer()
{
    m_shapesRoot->unref();
    m_rootNode->unref();
    delete m_quarterWidget;
    //Quarter::exit();
}

void QuarterViewer::initialize()
{
    //// Set initial view
    //m_quarterWidget->viewAll();
    //
    //// Ensure QuarterWidget displays correctly
    //this->setAttribute(Qt::WA_OpaquePaintEvent);
    //this->setAttribute(Qt::WA_PaintOnScreen);
    //
    //// Simplify background settings, remove code that might cause issues
    //
    //// Add obvious test lines with increased line width and color contrast
    //qDebug() << "QuarterViewer: Initializing with test geometry";
    //
    //// Add test lines to verify drawing functionality
    //SoSeparator *testSeparator = new SoSeparator();
    //
    //// Set material - use more vibrant color
    //SoMaterial *testMaterial = new SoMaterial();
    //testMaterial->diffuseColor.setValue(1.0, 0.0, 0.0); // Vibrant red
    //testMaterial->emissiveColor.setValue(0.5, 0.0, 0.0); // Add self-emission to improve visibility
    //testSeparator->addChild(testMaterial);
    //
    //// Set draw style - increase line width
    //SoDrawStyle *testDrawStyle = new SoDrawStyle();
    //testDrawStyle->lineWidth.setValue(5.0); // Thicker lines
    //testSeparator->addChild(testDrawStyle);
    //
    //// Create multiple lines to form a cross for better visibility
    //SoCoordinate3 *testCoords = new SoCoordinate3();
    //// Horizontal line
    //testCoords->point.set1Value(0, -100, 0, 0); // Left endpoint
    //testCoords->point.set1Value(1, 100, 0, 0);  // Right endpoint
    //// Vertical line
    //testCoords->point.set1Value(2, 0, -100, 0); // Bottom endpoint
    //testCoords->point.set1Value(3, 0, 100, 0);  // Top endpoint
    //testSeparator->addChild(testCoords);
    //
    //// Create line set
    //SoLineSet *testLineSet = new SoLineSet();
    //// Specify number of vertices per line
    //int numVertices[] = {2, 2}; // Two lines, each with 2 vertices
    //testLineSet->numVertices.setValues(0, 2, numVertices);
    //testSeparator->addChild(testLineSet);
    //
    //// Ensure test geometry is centered and visible in the scene
    //SoTransform *transform = new SoTransform();
    //transform->translation.setValue(0, 0, 0);
    //transform->scaleFactor.setValue(1, 1, 1);
    //testSeparator->insertChild(transform, 0);
    //
    //// Clear any existing shapes to ensure only test geometry is displayed
    //m_shapesRoot->removeAllChildren();
    //
    //// Add to scene
    //m_shapesRoot->addChild(testSeparator);
    //
    //// Add additional debug information
    //qDebug() << "QuarterViewer: Test cross added to scene with lines: horizontal from (-100,0,0) to (100,0,0) and vertical from (0,-100,0) to (0,100,0)";
    //qDebug() << "QuarterViewer: Camera position:" << m_rootNode->getChild(3)->getTypeId().getName(); // Check camera node
    //
    //// Force view update
    //m_quarterWidget->update();
    //this->update();
    qDebug() << "QuarterViewer: View updated after adding test geometry";
}

void QuarterViewer::addShape(const TopoDS_Shape &shape, const Quantity_Color &color, int lineStyle, int lineWidth)
{
    try {
        // Check if shape is valid
        if (shape.IsNull()) {
            qDebug() << "QuarterViewer: Attempting to add null shape, skipping";
            return;
        }
        
        // Convert OCCT shape to Coin3D node
        SoNode *node = shapeToCoinNode(shape, color, lineStyle, lineWidth);
        
        // If conversion successful, add to root node
        if (node) {
            m_shapesRoot->addChild(node);
            qDebug() << "QuarterViewer: Shape added to scene graph";
            emit viewUpdated();
        } else {
            qDebug() << "QuarterViewer: Failed to convert shape to Coin3D node";
        }
    } catch (const Standard_Failure &e) {
        qDebug() << "QuarterViewer: OCCT exception when adding shape:" << e.GetMessageString();
    } catch (...) {
        qDebug() << "QuarterViewer: Unknown exception when adding shape";
    }
}

void QuarterViewer::clearAllShapes()
{
    if (m_shapesRoot) {
        m_shapesRoot->removeAllChildren();
        qDebug() << "QuarterViewer: All shapes cleared from scene graph";
    }
    emit viewUpdated();
}

void QuarterViewer::render()
{
    // QuarterWidget doesn't have updateGL method, use update() instead
    m_quarterWidget->update();
}

void QuarterViewer::setCurrentMode(int mode)
{
    m_currentMode = mode;
}

gp_Pnt QuarterViewer::getPointFromScreen(const QPoint &screenPos)
{
    // Calculate the 3D point corresponding to the screen coordinates
    // This uses a simplified implementation, actual projects may need more complex ray casting
    
    // Get viewport size
    QSize viewportSize = m_quarterWidget->size();
    
    // Normalize coordinates to [-1, 1] range
    Standard_Real normalizedX = (2.0 * screenPos.x() / viewportSize.width()) - 1.0;
    Standard_Real normalizedY = 1.0 - (2.0 * screenPos.y() / viewportSize.height());
    
    // Assume a simple orthogonal projection
    // In actual applications, should use camera's projection matrix and view matrix
    Standard_Real scale = 1.0; // Scale factor
    Standard_Real z = 0.0;     // Fixed Z coordinate
    
    gp_Pnt result(normalizedX * scale * 50.0, normalizedY * scale * 50.0, z);
    return result;
}

gp_Pnt QuarterViewer::getPointFromScreen(int x, int y)
{
    return getPointFromScreen(QPoint(x, y));
}

void QuarterViewer::mousePressEvent(QMouseEvent *event)
{
    // Calculate 3D point and emit signal
    gp_Pnt point = getPointFromScreen(event->x(), event->y());
    emit mousePressed(point);
    
    // Save current mouse position
    m_lastMousePos = event->pos();
    
    qDebug() << "QuarterViewer: Mouse pressed at screen position" << event->pos() << ", 3D point" << point.X() << point.Y() << point.Z();
    
    // Call base class event handling
    QWidget::mousePressEvent(event);
    
    // QuarterWidget's processSoEvent requires SoEvent type, comment out incompatible call
    // Directly use Qt's event system and QuarterWidget's update() method to update view
    // if (m_quarterWidget) {
    //     m_quarterWidget->processSoEvent(event);
    // }
}

void QuarterViewer::mouseMoveEvent(QMouseEvent *event)
{
    // Calculate 3D point and emit signal
    gp_Pnt point = getPointFromScreen(event->x(), event->y());
    emit mouseMoved(point);
    
    qDebug() << "QuarterViewer: Mouse moved at screen position" << event->pos() << ", 3D point" << point.X() << point.Y() << point.Z();
    
    // Call base class event handling
    QWidget::mouseMoveEvent(event);
    
    // QuarterWidget's processSoEvent requires SoEvent type, comment out incompatible call
    // Directly use Qt's event system and QuarterWidget's update() method to update view
    // if (m_quarterWidget) {
    //     m_quarterWidget->processSoEvent(event);
    // }
    m_quarterWidget->update();
}

void QuarterViewer::mouseReleaseEvent(QMouseEvent *event)
{
    // Calculate 3D point and emit signal
    gp_Pnt point = getPointFromScreen(event->x(), event->y());
    emit mouseReleased(point);
    
    qDebug() << "QuarterViewer: Mouse released at screen position" << event->pos() << ", 3D point" << point.X() << point.Y() << point.Z();
    
    // Call base class event handling
    QWidget::mouseReleaseEvent(event);
    
    // QuarterWidget's processSoEvent requires SoEvent type, comment out incompatible call
    // Directly use Qt's event system and QuarterWidget's update() method to update view
    // if (m_quarterWidget) {
    //     m_quarterWidget->processSoEvent(event);
    // }
    m_quarterWidget->update();
}

SoNode *QuarterViewer::shapeToCoinNode(const TopoDS_Shape &shape, const Quantity_Color &color, int lineStyle, int lineWidth)
{
    SoSeparator *separator = new SoSeparator();
    separator->setName("ShapeNode");
    
    // Set material properties
    SoMaterial *material = new SoMaterial();
    Standard_Real r, g, b;
    r = color.Red();
    g = color.Green();
    b = color.Blue();
    material->diffuseColor.setValue(r, g, b);
    material->emissiveColor.setValue(r * 0.2, g * 0.2, b * 0.2); // Add some self-emission effect
    separator->addChild(material);
    
    // Set draw style
    SoDrawStyle *drawStyle = new SoDrawStyle();
    drawStyle->lineWidth.setValue(lineWidth);
    
    // Set different line patterns according to line style
    switch (lineStyle) {
    case 0: // Solid line
        drawStyle->linePattern.setValue(0xffff);
        break;
    case 1: // Dashed line
        drawStyle->linePattern.setValue(0xf0f0);
        break;
    case 2: // Dotted line
        drawStyle->linePattern.setValue(0x8888);
        break;
    default:
        drawStyle->linePattern.setValue(0xffff);
    }
    
    separator->addChild(drawStyle);
    
    // Create different geometry nodes according to shape type
    try {
        if (shape.IsNull()) {
            return nullptr;
        }
        
        SoCoordinate3 *coords = new SoCoordinate3();
        SoLineSet *lineSet = new SoLineSet();
        int pointIndex = 0;
        std::vector<int> numVertices;
        
        // Unified processing: iterate through all edges in the shape
        TopExp_Explorer edgeExplorer(shape, TopAbs_EDGE);
        
        while (edgeExplorer.More()) {
            TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());
            
            if (!edge.IsNull() && BRep_Tool::IsGeometric(edge)) {
                // Get two endpoints of the edge
                gp_Pnt start, end;
                TopExp_Explorer vertexExp(edge, TopAbs_VERTEX);
                
                if (vertexExp.More()) {
                    start = BRep_Tool::Pnt(TopoDS::Vertex(vertexExp.Current()));
                    vertexExp.Next();
                    if (vertexExp.More()) {
                        end = BRep_Tool::Pnt(TopoDS::Vertex(vertexExp.Current()));
                        
                        // Add two endpoints of the edge
                        coords->point.set1Value(pointIndex++, start.X(), start.Y(), start.Z());
                        coords->point.set1Value(pointIndex++, end.X(), end.Y(), end.Z());
                        numVertices.push_back(2); // Each edge has 2 vertices
                    }
                }
            }
            
            edgeExplorer.Next();
        }
        
        // If no edges, try to get vertices directly (handle point type)
        if (numVertices.empty()) {
            TopExp_Explorer vertexExp(shape, TopAbs_VERTEX);
            if (vertexExp.More()) {
                gp_Pnt p = BRep_Tool::Pnt(TopoDS::Vertex(vertexExp.Current()));
                
                // Create a cross to represent the point
                Standard_Real size = lineWidth * 2.0;
                coords->point.set1Value(pointIndex++, p.X() - size, p.Y(), p.Z());
                coords->point.set1Value(pointIndex++, p.X() + size, p.Y(), p.Z());
                numVertices.push_back(2); // Horizontal line
                
                coords->point.set1Value(pointIndex++, p.X(), p.Y() - size, p.Z());
                coords->point.set1Value(pointIndex++, p.X(), p.Y() + size, p.Z());
                numVertices.push_back(2); // Vertical line
            }
        }
        
        // If there are vertex data, add to scene
        if (!numVertices.empty()) {
            lineSet->numVertices.setValues(0, numVertices.size(), numVertices.data());
            separator->addChild(coords);
            separator->addChild(lineSet);
            qDebug() << "QuarterViewer: Added" << numVertices.size() << "edges," << pointIndex << "points";
        }
        else {
            qDebug() << "QuarterViewer: No vertices found in shape";
            // At least add a visible point to avoid empty node
            coords->point.set1Value(0, 0, 0, 0);
            SoPointSet *pointSet = new SoPointSet();
            separator->addChild(coords);
            separator->addChild(pointSet);
        }
    } catch (const Standard_Failure &e) {
        qDebug() << "QuarterViewer: OCCT exception in shapeToCoinNode:" << e.GetMessageString();
        // Add a visible error marker
        SoCoordinate3 *coords = new SoCoordinate3();
        coords->point.set1Value(0, 0, 0, 0);
        SoPointSet *pointSet = new SoPointSet();
        separator->addChild(coords);
        separator->addChild(pointSet);
    }
    
    return separator;
}