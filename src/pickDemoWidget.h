#ifndef PICKDEMOWIDGET_H
#define PICKDEMOWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
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

#include <Quarter/QuarterWidget.h>

class pickDemoWidget : public QWidget
{
    Q_OBJECT

public:
    pickDemoWidget(QWidget *parent = nullptr);
    ~pickDemoWidget();

protected:
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void setupUI();
    void createCube();
    void performPick(int x, int y);
    void updateHighlight();

    // Quarter widget for 3D rendering
    SIM::Coin3D::Quarter::QuarterWidget* m_quarterWidget;

    // Scene graph nodes
    SoSeparator* m_root;
    SoSeparator* m_modelRoot;
    SoPerspectiveCamera* m_camera;
    SoSeparator* m_pickRoot;
    SoSeparator* m_highlightRoot;

    // Cube components
    SoCoordinate3* m_coords;
    SoFaceSet* m_faceSet;

    // Highlight materials
    SoMaterial* m_vertexHighlightMat;
    SoMaterial* m_edgeHighlightMat;
    SoMaterial* m_faceHighlightMat;

    // Current highlighted elements
    int m_highlightedVertex;
    int m_highlightedEdge;
    int m_highlightedFace;

    // Cube vertex, edge, and face data
    struct CubeData {
        SbVec3f vertices[8];
        struct { int v1, v2; } edges[12];
        struct { int v1, v2, v3, v4; } faces[6];
    } m_cubeData;
};

#endif // PICKDEMOWIDGET_H
